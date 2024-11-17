#include "foxtalk_tuple.h"
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>

#include <foxtalk_handler.hpp>
// #include <debug_utils.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <fstream>
#include <vector>

constexpr int NUM_BUFFERS = 4;
class RawCameraCaptureHandler : public Handler
{ 
  int fd = -1;
  bool has_setup_buffers = false;

  uint8_t* buffers[NUM_BUFFERS]{};
  v4l2_buffer buffer_structs[NUM_BUFFERS]{};
  bool available_buffers[NUM_BUFFERS]{};
  const v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int current_buffer = -1;
 
  bool pollme = true;

public:
  void close_stream() {

    if (ioctl(fd, VIDIOC_STREAMOFF, &buf_type) == -1) {
      std::cerr << "Error turning stream OFF?? : " << strerror(errno) << std::endl;
    }

    has_setup_buffers = false;
    current_buffer = -1;
    for (auto i = 0; i < NUM_BUFFERS; i++) {
      munmap(buffers[i], buffer_structs[i].length);
      buffer_structs[i] = {};
      buffers[i] = nullptr;
      available_buffers[i] = true;
    }
    close(fd);  
    fd = -1;
    // Let the camera chill for a (tenth of a) sec
    usleep(100000);
  }
  ~RawCameraCaptureHandler() {
    // std::cout << "Raw camera capture dropping the fd: " << fd << std::endl;
    close_stream();
  }
  bool poll() override {
    if (!pollme) {
      return false;
    }
    if (!has_setup_buffers) {
      return false; 
    }
    
    for (int i = 0; i < NUM_BUFFERS; i++) {
      if (!available_buffers[i]) {
        continue;
      }

      auto res = ioctl(fd, VIDIOC_DQBUF, buffer_structs + i);

      if (res == 0) {
        available_buffers[i] = false;
        current_buffer = i;
        return true;
      } else {
        // std::cout << "Buffer index " << i << " dqbuf returned " << strerror(errno) << std::endl;
        // usleep(500000);
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
      std::cerr << "Failed to open camera " << camera << std::endl;
      return false;
    }

    struct v4l2_capability cap {};
    ioctl(fd, VIDIOC_QUERYCAP, &cap);

    if (!(cap.capabilities & buf_type)) {
      std::cerr << "Video camera " << camera << " does not have the V4L2_BUF_TYPE_VIDEO_CAPTURE capability!" << std::endl; 
      return false;
    }
 
    struct v4l2_format fmt {};

    fmt.type = buf_type;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixel_format;

    // Should get this from ioctl instead of assuming

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        std::cerr << "Error setting video format for " << camera << ": " << strerror(errno) << std::endl;
        return false;
    }

    struct v4l2_requestbuffers req {};


    // Get capabilities from ioctl, instead of assuming
    req.count = NUM_BUFFERS; // I think this is just four different buffers containing all frame data

    req.type = buf_type;
    req.memory = V4L2_MEMORY_MMAP;
    

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        std::cerr << "Error requesting video buffers: " << strerror(errno) << std::endl;
        return false;
    }
    // std::cout << std::boolalpha
    //           << "has mmap: " << (bool)(req.capabilities & V4L2_BUF_CAP_SUPPORTS_MMAP) 
    //           << std::endl
    //           << "has userptr: " << (bool)(req.capabilities & V4L2_BUF_CAP_SUPPORTS_USERPTR) 
    //           << std::endl
    //           << "has max num buffers: " << (bool)(req.capabilities & V4L2_BUF_CAP_SUPPORTS_MAX_NUM_BUFFERS) 
    //           << std::endl
    //           << "has request support: " << (bool)(req.capabilities & V4L2_BUF_CAP_SUPPORTS_REQUESTS) 
    //           << std::endl
    //           << "has dma buffer: " << (bool)(req.capabilities & V4L2_BUF_CAP_SUPPORTS_DMABUF) 
    //           << std::endl;


    for (int i = 0; i < req.count; i++) {
        buffer_structs[i].type = buf_type;
        buffer_structs[i].memory = V4L2_MEMORY_MMAP; 
        buffer_structs[i].index = i;
        size_t length = 0;
        off_t offset = 0;
        // buf.m.userptr = 

        if (ioctl(fd, VIDIOC_QUERYBUF, buffer_structs + i) == -1) {
          std::cerr << "Error getting frame data for buffer " << strerror(errno) << std::endl;
          return false;
        }
// 
        // std::cout << "Buffer len: " << buf.length << std::endl;
        length = buffer_structs[i].length;
        offset = buffer_structs[i].m.offset;

        // std::cout << "length: " << length << " offset: " << offset << std::endl;

        buffers[i] =
                static_cast<uint8_t *>(mmap(nullptr /* start anywhere */,
                     length,
                     PROT_READ /* required */,
                     MAP_SHARED /* recommended */,
                     fd, offset));
        if (buffers[0] == MAP_FAILED) {
          std::cerr << "Error mmaping frame data for buffer " << i << " | " << strerror(errno) << std::endl;
          return false;
        }
        
    }

  
    if (ioctl(fd, VIDIOC_STREAMON, &buf_type) == -1) {
      std::cerr << "Error turning stream on: " << strerror(errno) << std::endl;
      return false;
    }

    
    for (int i = 0; i < NUM_BUFFERS; i++) {
      // buffer_structs[i] === *(buffer_structs + i)
      if (ioctl(fd, VIDIOC_QBUF, buffer_structs + i) == -1) {
        std::cerr << "Error queueing buffer " << i << " | " << strerror(errno) << std::endl;
        return false;
      }
      available_buffers[i] = true;
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
    auto q = queryResults[0];
    auto camera = q.at<std::string>(2).value();
    auto pixel_format = q.at<uint64_t>(4).value();
    auto width = q.at<uint64_t>(6).value();
    auto height = q.at<uint64_t>(8).value();
    if (!has_setup_buffers) {
      auto fps = q.at<double>(10).value();

      if(setup_buffers(camera, pixel_format, width, height, fps)) {
        has_setup_buffers = true;
      } else {
        std::cerr << "Error setting up buffers!" << std::endl;

        return;
      }
    }
    if (current_buffer < 0) {
      return;
    }
    // now, some buffer has data!

    // std::cout << "CLAIM! Buffer " << current_buffer << " contained " << buffer_structs[current_buffer].bytesused << " bytes used " << std::endl;
    // Claim /dev/video0 has image Cptr...
    // auto frame = std::vector<uint8_t>(
    //   buffers[current_buffer], 
    //   (buffers[current_buffer]) + buffer_structs[current_buffer].length);
    
    claim({ 
      {{camera},
      {"with pixel format"}, 
      {pixel_format},
      {"with resolution width"},
      {width}, 
      {"with resolution height"}, 
      {height},
      {"has image"},
      {buffers[current_buffer]},
      {"at index"},
      {current_buffer},
      {"for frame #"},
      {(uint64_t)buffer_structs[current_buffer].sequence}
      }});
    current_buffer = -1;
  }


  void free_tuple(const Tuple &t) override {
    auto buffer_index = t.at<int64_t>(10).value();
    // if the tuple matches "/dev/video0 has image Cptr..."
    // Requeue the buffer pointed to @ Cptr
    // std::cout << "Requing buffer # " << buffer_index << std::endl;

    // TODO: Bounds check on buffer index
    buffer_structs[buffer_index].flags = 0;
    buffer_structs[buffer_index].reserved2 = 0;
    buffer_structs[buffer_index].reserved = 0;
    if (ioctl(fd, VIDIOC_QBUF, buffer_structs + buffer_index) == -1) {
      std::cerr << "Error queueing buffer " << buffer_index << " | " << strerror(errno) << std::endl;
    }
    available_buffers[buffer_index] = true;
    //std::cout << t << std::endl;  
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