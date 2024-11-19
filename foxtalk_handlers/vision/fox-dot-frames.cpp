// pkg-config: opencv4
// cppstd: 23

#include "foxtalk_tuple.h"
#include "opencv2/core/hal/interface.h"
#include <cstdint>
#include <foxtalk_handler.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <string>

class DotHandler : public Handler
{
  cv::Ptr<cv::SimpleBlobDetector> blob;

protected:
    void handle(const std::vector<Tuple> &queryResults) override
    {
      void* image_buffer = nullptr;
      int width, height;

      // Find the camera info for the rest of the handler.
      for(auto q : queryResults) {
        auto maybe_cam = q.at<std::string>(1);
        if(maybe_cam.has_value() && maybe_cam.value() == "with pixel format") {
          width = static_cast<int>(q.at<uint64_t>(4).value());
          height = static_cast<int>(q.at<uint64_t>(6).value());
          image_buffer = q.at<void *>(8).value();
        }
      }

      if(image_buffer == nullptr) return;
      // std::cout << "Got w " << width << " h " << height << std::endl;

      cv::Mat img(height, width, CV_8UC2, image_buffer);

      cv::Mat color, lab;
      cv::cvtColor(img, color, cv::COLOR_YUV2RGB_YUY2);
      cv::cvtColor(color, lab, cv::COLOR_RGB2Lab);

      // cv::Mat channels[3] {};
      // cv::split(lab, channels);

      // cv::Mat debug;
      // cv::cvtColor(channels[1], debug, cv::COLOR_GRAY2RGB);
      // std::vector<uint8_t> debug_img((uint8_t*)debug.data, (uint8_t*)debug.dataend);

      // claim({{
      //   {"cv debug image"},
      //   {debug_img},
      //   {"width"},
      //   {width},
      //   {"height"},
      //   {height}
      //   }});
    }

    void init() override
    {
        cv::SimpleBlobDetector::Params params {};

        claim({{{"camera"}, TupleNoun::query(),
        {"has dot at"},
          {"x"}, TupleNoun::query(),
          {"y"}, TupleNoun::query(),
          {"size"}, TupleNoun::query()
        }});

        claim({
          {TupleNoun::query(),
          {"with pixel format"},
          TupleNoun::query(),
          {"with resolution width"},
          TupleNoun::query(),
          {"with resolution height"},
          TupleNoun::query(),
          {"has image"},
          TupleNoun::query(),
          TupleNoun::query(),
          TupleNoun::query(),
          TupleNoun::query(),
          TupleNoun::query(),
        }});
    }
};

FOXTALK_FFI_HANDLER_REG(DotHandler);