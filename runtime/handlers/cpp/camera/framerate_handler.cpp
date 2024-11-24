
#include <foxtalk_handler.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>

class FrameRateHandler : public Handler
{
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    for (auto q: queryResults) {
      // log_debug(q);
      auto camera = q.at<std::string>(0).value();
      auto flags =  q.at<uint64_t>(4).value();
      auto pixel_format = q.at<uint64_t>(2).value();
      auto width = q.at<uint64_t>(6).value();
      auto height = q.at<uint64_t>(8).value();
      int fd = open(camera.c_str(), O_NONBLOCK);
      if (fd < 0) {
        log_error("Failed to open camera " << camera);
        return;
      }

      // Get the supported frame intervals
      struct v4l2_frmivalenum frmival {};
      frmival.width = width; // Set width
      frmival.height = height; // Set height
      frmival.pixel_format = pixel_format; // Set pixel format

      for (frmival.index = 0; ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0; frmival.index++) {

        auto fps = (double)frmival.discrete.denominator / (double)frmival.discrete.numerator;
        claim({{
          {camera},
          {"has pixel format"},
          {(uint64_t)pixel_format},
          {"with flags"},
          {flags},
          {"with resolution width"},
          {(uint64_t)width},
          {"with resolution height"},
          {(uint64_t)height},
          {"with fps"},
          {fps},}});

          // log_debug(frmival.discrete.numerator << "/" << frmival.discrete.denominator << " fps");
      }
      close(fd);
    }
  }
  void init() override
  {
    claim({{
      TupleNoun::query(),
      {"has pixel format"},
      TupleNoun::query(),
      {"with flags"},
      TupleNoun::query(),
      {"with resolution width"},
      TupleNoun::query(),
      {"with resolution height"},
      TupleNoun::query()
      }});
  }

};

FOXTALK_FFI_HANDLER_REG(FrameRateHandler);
