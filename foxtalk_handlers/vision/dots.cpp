// pkg-config: opencv4
// cppstd: 23

#include "opencv2/core/hal/interface.h"
#include <foxtalk_handler.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

class HuskyHandler : public Handler
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
      // auto image_buffer = q.at<std::vector<uint8_t>>(8).value();
      auto image_buffer = q.at<void *>(8).value();


      cv::Mat img(height, width, CV_8UC1, image_buffer, width * 4);

      cv::Mat gray;
      cv::cvtColor(img, gray, cv::COLOR_YUV2GRAY_420);

      cv::GaussianBlur(gray, gray, {5, 5}, 4);

      std::vector<cv::KeyPoint> keypoints;
      blob->detect(gray, keypoints);

      for(auto kp : keypoints) {
        claim({{{"camera"}, {camera}, {"has blob at"},
          {"x"}, {kp.pt.x},
          {"y"}, {kp.pt.y},
          {"size"}, {kp.size}
        }});
      }

    }

    void free_tuple(const Tuple &o) override
    {

        std::cout << "free tuple in husky handler: " << o << std::endl;
    }

    void init() override
    {
        cv::SimpleBlobDetector::Params params {};
        params.minArea = 150;
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
          TupleNoun::prefix(),
        }});
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);