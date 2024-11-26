#include "foxtalk_tuple.h"
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>

#include <foxtalk_handler.hpp>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>


struct cam_buffer {
  void* mem;
  size_t length;
};

constexpr int NUM_BUFFERS = 4;
constexpr v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

class RawCameraCaptureHandler : public Handler
{
  int fd = -1;
  bool has_setup_buffers = false;

  cam_buffer buffers[NUM_BUFFERS]{};
  bool currently_queued_buffers[NUM_BUFFERS]{};

  int current_buffer_index = -1;

public:
  void close_stream() {

    if (ioctl(fd, VIDIOC_STREAMOFF, &buf_type) == -1) {
      err << "Error turning stream OFF?? : " << strerror(errno) << end;
    }
    has_setup_buffers = false;
    current_buffer_index = -1;
    for (auto & buffer : buffers) {
      munmap(buffer.mem, buffer.length);
    }
    close(fd);
    fd = -1;
    // Let the camera chill for a (tenth of a) sec
    usleep(100000);
  }
  ~RawCameraCaptureHandler() {
    // debug << "Raw camera capture dropping the fd: " << fd << end;
    close_stream();
  }
  bool poll() override {
    if (!has_setup_buffers) {
      return false;
    }

    for (int i = 0; i < NUM_BUFFERS; i++) {
      v4l2_buffer buff {};
      buff.index = i;
      buff.type = buf_type;

      auto res = ioctl(fd, VIDIOC_QUERYBUF, &buff);
      if(res == 0 && buff.flags & V4L2_BUF_FLAG_DONE) {
        current_buffer_index = i;
        return true;
      }
    }
    return false;
  }

protected:

  // TODO: void that throws exceptions, not a bool
  bool setup_buffers(
    const std::string& camera,
    uint64_t pixel_format,
    uint64_t width,
    uint64_t height,
    double fps)
  {

    fd = open(camera.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
      err << "Failed to open camera " << camera << end;
      return false;
    }

    struct v4l2_capability cap {};
    ioctl(fd, VIDIOC_QUERYCAP, &cap);

    if (!(cap.capabilities & buf_type)) {
      err << "Video camera " << camera << " does not have the V4L2_BUF_TYPE_VIDEO_CAPTURE capability!" << end;
      return false;
    }

    struct v4l2_format fmt {};

    fmt.type = buf_type;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixel_format;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        err << "Error setting video format for " << camera << ": " << strerror(errno) << end;
        return false;
    }

    struct v4l2_requestbuffers req {};

    req.count = NUM_BUFFERS; 
    req.type = buf_type;
    req.memory = V4L2_MEMORY_MMAP;


    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        err << "Error requesting video buffers: " << strerror(errno) << end;
        return false;
    }

    for (int i = 0; i < req.count; i++) {
    
      v4l2_buffer buf {};
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      size_t length = 0;
      off_t offset = 0;

      if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
        err << "Error getting frame data for buffer " << strerror(errno) << end;
        return false;
      }

      length = buf.length;
      offset = buf.m.offset;

      buffers[i].mem = mmap(nullptr /* start anywhere */,
                    length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, offset);

      buffers[i].length = length;

      if (buffers[i].mem == MAP_FAILED) {
        err << "Error mmaping frame data for buffer " << i << " | " << strerror(errno) << end;
        return false;
      }
      

    }


    if (ioctl(fd, VIDIOC_STREAMON, &buf_type) == -1) {
      err << "Error turning stream on: " << strerror(errno) << end;
      return false;
    }


    for (int i = 0; i < NUM_BUFFERS; i++) {
      
      v4l2_buffer buf {};
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;

      if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        err << "[setup_buffers] Error queueing buffer " << i << " | " << strerror(errno) << end;
        return false;
      } 
      // debug << "Setting queued to true for " << i << end;
      currently_queued_buffers[i] = true;
      
    }
    return true;
  }

  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() == 0) {
      if (has_setup_buffers) {
        close_stream();
      }
    }
    if (queryResults.size() != 1) {
      return;
    }
    const auto& q = queryResults[0];
    auto camera = q.at<std::string>(2).value();
    auto pixel_format = q.at<uint64_t>(4).value();
    auto width = q.at<uint64_t>(6).value();
    auto height = q.at<uint64_t>(8).value();
    if (!has_setup_buffers) {
      auto fps = q.at<double>(10).value();

      if(setup_buffers(camera, pixel_format, width, height, fps)) {
        has_setup_buffers = true;
      } else {
        err << "Error setting up buffers!" << end;
        return;
      }
    }

    if (current_buffer_index < 0) {
      return;
    }

    v4l2_buffer buf {};
    buf.index = current_buffer_index;
    buf.type = buf_type;

    auto res = ioctl(fd, VIDIOC_DQBUF, &buf);
    if(res == 0) {
      // debug << "Setting queued to false for " << current_buffer_index << end;
      currently_queued_buffers[current_buffer_index] = false;
      claim({
        {{camera},
        {"with pixel format"},
        {pixel_format},
        {"with resolution width"},
        {width},
        {"with resolution height"},
        {height},
        {"has image"},
        {buffers[current_buffer_index].mem},
        {"at index"},
        {current_buffer_index},
        {"for frame #"},
        {(uint64_t)buf.sequence}
        }});
    }
    else {
      err << "[handle] Error dequeueing buffer " << current_buffer_index << " | " << strerror(errno) << end;
    }
    

    current_buffer_index = -1;
  }


  void free_tuple(const Tuple &t) override {
    auto buffer_index = t.at<int64_t>(10).value();
    // if the tuple matches "/dev/video0 has image Cptr..."
    // Requeue the buffer pointed to @ Cptr
    // debug << "Requing buffer # " << buffer_index << end;

    // TODO: Bounds check on buffer index
    
    if (!currently_queued_buffers[buffer_index]) {
      v4l2_buffer buf {};
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = buffer_index;

      if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        err << "[free_tuple] Error queueing buffer " << buffer_index << " | " << strerror(errno) << end;
        return;
      }
      // debug << "Setting queued to true for " << buffer_index << end;
      currently_queued_buffers[buffer_index] = true;
      
    }
    
  }

  void init() override {

    claim({{
      {"chosen foxtalk camera"},
      {"is"},
      {TupleNoun::query()},
      {"with pixel format"},
      {TupleNoun::query()},
      {"with resolution width"},
      {TupleNoun::query()},
      {"with resolution height"},
      {TupleNoun::query()},
      {"with fps"},
      {TupleNoun::query()},
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(RawCameraCaptureHandler);