#include <foxtalk_handler.hpp>

#include <iostream>

#include <string.h> // this is where strerror lives, apparently.
#include <fcntl.h>
#include <functional>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

class WebcamHandler : public Handler
{

  void addCameraDevice(std::string camName)
  {
    int fd = open(camName.c_str(), O_NONBLOCK);
    std::cout << fd << std::endl;
    // if (fd < 0)
    // {
    //   throw std::runtime_error(std::format("ioctl failed with error: {0}", strerror(errno)));
    // }

    if (fd >= 0)
    {
      claim({{{camName}, {"is a"}, {"camera"}}});
    }

    // struct v4l2_fmtdesc fmtdesc
    // {
    //   .index = 0,
    //   .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    // };
    // v4lEnumerate<struct v4l2_fmtdesc, VIDIOC_ENUM_FMT>(fd, fmtdesc, [&db, &camName, fd](auto desc)
    //                                                    {
    //      std::cout << "Enum'd format [" << desc.index << "]: " << desc.description << std::endl;

    //      db.claim(desc.description, "is a", "format");
    //      db.claim(desc.description, "has the id", (void*)desc.pixelformat); // omg.
    //      db.claim(camName, "has the format", desc.description);

    //      struct v4l2_frmsizeenum framesize {
    //              .index = 0,
    //              .pixel_format = desc.pixelformat
    //      };
    //      v4lEnumerate<struct v4l2_frmsizeenum, VIDIOC_ENUM_FRAMESIZES>(fd, framesize, [&db, &camName](auto desc) {
    //          if(desc.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
    //              // Hm. How do I say "has resolution in pixel format...?"
    //              // Also, I'd like this to be un-nested. The presence of a camera should trigger
    //              // the finding of pixel formats; the presence of pixel formats should find
    //              db.claim(camName, "has resolution", new std::pair { desc.discrete.width, desc.discrete.height });

    //              std::cout << "    Discrete: " << desc.discrete.width << "x" << desc.discrete.height << std::endl;
    //          } else {
    //              throw std::runtime_error("Unimplemented framesize handler...");
    //          }
    //      }); });
  }

  // output: <"/dev/video1", "is a", "camera">;

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    for (auto q: queryResults) {
      auto camera = q.at<std::string>(1).value();
      addCameraDevice(camera);
    }
  }
  void init() override
  {
    claim({{{"camera"}, TupleNoun::query(), {"state changed"}}});
  }

};

FOXTALK_FFI_HANDLER_REG(WebcamHandler);
