// pkg-config: opencv4
// cppstd: 23

#include "opencv2/core/hal/interface.h"
#include <cstdint>
#include <foxtalk_handler.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

class DotHandler : public Handler
{
  cv::Ptr<cv::SimpleBlobDetector> blob;

protected:
    void handle(const std::vector<Tuple> &queryResults) override
    {
      if(queryResults.size() != 1) { return; }

      auto q = queryResults[0];
      auto camera = q.at<std::string>(0).value();
      auto pixel_format = q.at<uint64_t>(2).value();
      auto width = q.at<uint64_t>(4).value();
      auto height = q.at<uint64_t>(6).value();

      auto image_buffer = q.at<void *>(8).value();

      cv::Mat img(height, width, CV_8UC2, image_buffer);

      cv::Mat gray;
      cv::cvtColor(img, gray, cv::COLOR_YUV2GRAY_YUY2);
      cv::GaussianBlur(gray, gray, {3, 3}, 4);

      // cv::Mat debug;
      // cv::cvtColor(gray, debug, cv::COLOR_GRAY2RGB);
      // std::vector<uint8_t> debug_img((uint8_t*)debug.data, (uint8_t*)debug.dataend);

      // claim({{
      //   {"cv debug image"},
      //   {debug_img},
      //   {"width"},
      //   {width},
      //   {"height"},
      //   {height}
      //   }});

      std::vector<cv::KeyPoint> keypoints;
      blob->detect(gray, keypoints);

      for(auto kp : keypoints) {
        claim({{{"camera"}, {camera},
        {"has dot at"},
          {"x"}, {kp.pt.x},
          {"y"}, {kp.pt.y},
          {"size"}, {kp.size}
        }});
      }

      // claim({{{"camera"}, {camera},
      // {"has dot at"},
      //   {"x"}, {100.0},
      //   {"y"}, {100.0},
      //   {"size"}, {30.0}
      // }});
    }

    void init() override
    {
        cv::SimpleBlobDetector::Params params {};

        // Big dots
        // params.minArea = 150;

        // Tiny dots
        params.minArea = 70;
        params.maxArea = 600;

        params.filterByCircularity = true;
        params.minCircularity = 0.75;

        blob = cv::SimpleBlobDetector::create(params);

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