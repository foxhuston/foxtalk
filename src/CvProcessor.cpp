#include "opencv2/core/base.hpp"
#include "opencv2/core/hal/interface.h"
#include "opencv2/core/matx.hpp"
#include "opencv2/core/types.hpp"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <iostream>
#include <limits>
#include <sstream>

#include <array>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/features2d.hpp>
#include <string>


static bool stop = false;
static cv::Ptr<cv::SimpleBlobDetector> blob;

static std::array<cv::Vec3b, 4> colorsBgr = {{
  { 0,   179, 0     },   // Green
  { 102, 0,   76    },   // Purple
  { 25,  127, 229   },   // Yellow
  { 0,   0,   255   }    // Red
}};

static std::array<cv::Vec3f, 4> colorsHsv = {{
  { 72.0f,   147.0f, 153.0f     },   // Green
  { 127.0f, 127.0f, 127.0f    },   // Purple
  { 9.0f,  176.0f, 200.0f   },   // Yellow
  { 178.0f,   184.0f,   226.0f   }    // Red
}};

constexpr int COLOR_THRESHOLD = 50;

void printColor(cv::Vec3f bgr, cv::Vec3f hsv) {
  std::cout
    << "\x1B[48;2;"
    << (uint32_t)bgr[2]
    << ";" 
    << (uint32_t)bgr[1]
    << ";" 
    << (uint32_t)bgr[0]
    << "m"
    << "<"
    << hsv[0] << ", "
    << hsv[1] << ", "
    << hsv[2] << ">"
    << "\x1B[0m";
}

extern "C" void process_image(
    cv::Mat& cameraFrame
    , cv::freetype::FreeType2* _cv_ft2
) {
  if(stop) return;
  if(blob == nullptr) {
    cv::SimpleBlobDetector::Params params {};
    params.filterByCircularity = true;
    params.minCircularity = 0.75;

    blob = cv::SimpleBlobDetector::create(params);
    std::cout << "Made Detector..." << std::endl;
  }

  try {
    cv::Mat gray, hsv;

    cv::cvtColor(cameraFrame, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(cameraFrame, hsv, cv::COLOR_BGR2HSV);

    hsv.convertTo(hsv, CV_32F);


    // Cool laplacian visualizer
    /* cv::Mat dst, abs_dst; */
    /* cv::Laplacian(gray, dst, CV_16S, 3, 1.0, 0.0, cv::BORDER_DEFAULT); */

    /* cv::convertScaleAbs(dst, abs_dst); */
    /* cv::applyColorMap(abs_dst, cameraFrame, cv::COLORMAP_PARULA); */
    /* return; */

    // Corner detection??
    /* cv::Mat harris; */
    /* cv::cornerHarris(gray, harris, 2, 3, 0.4); */
    /* cv::dilate(harris, harris, harris); // Marsha, Marsha, Marsha! */

    /* cv::cvtColor(harris, cameraFrame, cv::COLOR_GRAY2BGR); */
    /* return; */


    // Blob Detection
    std::vector<cv::KeyPoint> keypoints;
    blob->detect(gray, keypoints);

    /* std::cout << cameraFrame.type() << std::endl; */

    std::vector<cv::KeyPoint> filteredKeypoints;
    for(auto kp : keypoints) {
      cv::Vec3b blobBgr = cameraFrame.at<cv::Vec3b>(kp.pt);
      cv::Vec3f blobHsv = hsv.at<cv::Vec3f>(kp.pt);

      uint32_t min_n = std::numeric_limits<uint32_t>::max();
      cv::Vec3f minHsv = { 127, 127, 127 };
      cv::Vec3f minBgr = { 127, 127, 127 };

      for(int i = 0; i < colorsBgr.size(); i++) {
        cv::Vec3b bgr = colorsBgr[i];
        cv::Vec3f hsv = colorsHsv[i];

        /* cv::Vec3f diff = hsv - blobHsv; */
        /* std::cout << "\t diff = "; */
        /* printColor(diff, diff); */
        /* std::cout << std::endl; */

        float n = cv::norm(hsv - blobHsv, cv::NORM_L2);
        // Hue Only?
        /* float n = std::abs(hsv[0] - blobHsv[0]); */

        /* std::cout << "\t norm("; */
        /* printColor(bgr, hsv); */
        /* std::cout << " - "; */
        /* printColor(blobBgr, blobHsv); */
        /* std::cout << ") = " << n << std::endl; */

        if(n < min_n) {
          min_n = n;
          minHsv = hsv;
          minBgr = bgr;
        }
      }

      if(min_n < COLOR_THRESHOLD) {
        /* std::cout << "Found Blob Color: "; */
        /* printColor(blobBgr, blobHsv); */
        /* std::cout << " for bin "; */
        /* printColor(minBgr, minHsv); */
        /* std::cout << std::endl; */

        cv::circle(
          cameraFrame
          , kp.pt
          , 20
          , minBgr
          , 5
        );
      } else {
        /* std::cout << "MISSED Blob Color: "; */
        /* printColor(blobBgr, blobHsv); */
        /* std::cout << "(min n was " << min_n << ")" << std::endl; */
      }
    }


    /* cv::drawKeypoints(cameraFrame, keypoints, cameraFrame); */

    return;

    /* auto mult = 2; */
    /* auto alpha = 2.0; */
    /* auto beta = -100; */

    /* /1* cv::resize(gray, gray, cv::Size { static_cast<int>(_camWidth) / mult, static_cast<int>(_camHeight) / mult }, cv::INTER_CUBIC); *1/ */

    /* // cv::GaussianBlur(gray, gray, cv::Size {9, 9}, 2, 2); */
    /* cv::Laplacian(gray, gray, 1, 3); */

    /* cv::cvtColor(gray, cameraFrame, cv::COLOR_GRAY2BGR); */
    /* /1* return; *1/ */

    /* return; */
    /* std::vector<cv::Vec3f> circles; */

    /* cv::HoughCircles( */
    /*   gray */
    /*   , circles */
    /*   , cv::HOUGH_GRADIENT_ALT */
    /*   , 10 */
    /*   , 1 // 7 */
    /*   , 300 */
    /*   , 0.9 */
    /*   , 1 */
    /*   , 12 */
    /* ); */

    /* std::sort(circles.begin(), circles.end(), [](auto a, auto b) { */
    /*     return a[1] > b[1]; */
    /* }); */

    /* for(auto i = 0; i < circles.size(); i++) { */
    /*   auto c = circles[i]; */
    /*   auto color = CV_RGB(255.0, 0.0, 0.0); */

    /*   cv::Point center(c[0], c[1]); */

    /*   if(i == 0) { */
    /*     color = CV_RGB(0.0, 255.0, 255.0); */

    /*     std::stringstream ss; */
    /*     ss << c[2]; */

    /*     _cv_ft2->putText( */
    /*         cameraFrame */
    /*         , ss.str() */
    /*         , center */
    /*         , 60 */
    /*         , color */
    /*         , -1 // negative thickness fills the text. */
    /*         , cv::LINE_AA */
    /*         , true); */
    /*   } */
    /*   cv::circle(cameraFrame, center, c[2], color, 2); */
    /* } */

  } catch (cv::Exception cve) {
    std::cerr << "Caught exception! " << cve.what() << std::endl;
    stop = true;
    return;
  }
}
