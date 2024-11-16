#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>

class ResolutionHandler : public Handler
{
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    for (auto q: queryResults) {
      auto camera = q.at<std::string>(0).value();
      auto pixel_format = q.at<uint64_t>(2).value();
      int fd = open(camera.c_str(), O_NONBLOCK);
      if (fd < 0) {
        std::cerr << "Failed to open camera " << camera << std::endl;
        return;
      }

      struct v4l2_frmsizeenum frmsize{};
      memset(&frmsize, 0, sizeof(frmsize));
      frmsize.pixel_format = (uint32_t)pixel_format; 

      while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {

          claim({{ 
            {camera}, 
            {"has pixel format"},
            {(uint64_t)pixel_format}, 
            {"with resolution width"},
            {(uint64_t)frmsize.discrete.width},
            {"with resolution height"},
            {(uint64_t)frmsize.discrete.height},}});
              // std::cout << "Resolution: " << frmsize.discrete.width << "x" << frmsize.discrete.height << "\n";
        }
        frmsize.index++;
      }
      close(fd);
    }
  }
  void init() override
  {
    claim({{TupleNoun::query(), {"has pixel format"}, {TupleNoun::query()}, {TupleNoun::prefix()}}});
  }

};

FOXTALK_FFI_HANDLER_REG(ResolutionHandler);
