#include <foxtalk_handler.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

class PixelFormatHandler : public Handler
{
  bool retry_fd_open = false;
public:
  bool poll() override {
    return retry_fd_open;
  }
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    for (auto q: queryResults) {
      auto camera = q.at<std::string>(0).value();
      int fd = open(camera.c_str(), O_NONBLOCK);
      if (fd < 0) {
        retry_fd_open = true;
        return;
      }
      retry_fd_open = false; 
      struct v4l2_fmtdesc fmt{};
      memset(&fmt, 0, sizeof(fmt));
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
        auto d = fmt.description;
        std::string desc = std::string(d, d + sizeof(d));
        claim({{{camera}, {"has pixel format"}, {(uint64_t)fmt.pixelformat}, {"with description"}, {desc}}});
        fmt.index++;
      }
      close(fd);
    }
  }
  void init() override
  {
    claim({{TupleNoun::query(), {"is a"}, {"camera device"}}});
  }

};

FOXTALK_FFI_HANDLER_REG(PixelFormatHandler);
