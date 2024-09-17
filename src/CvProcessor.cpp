#include "opencv2/core/base.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/freetype.hpp>


static bool stop = false;

extern "C" void process_image(
    cv::Mat& cameraFrame
    , uint32_t _camWidth
    , uint32_t _camHeight
    , cv::freetype::FreeType2* _cv_ft2
) {
  if(stop) return;

  try {
    cv::Mat gray;
    cv::cvtColor(cameraFrame, gray, cv::COLOR_BGR2GRAY);


    // Cool laplacian visualizer
    cv::Mat dst, abs_dst;
    cv::Laplacian(gray, dst, CV_16S, 3, 1.0, 0.0, cv::BORDER_DEFAULT);

    cv::convertScaleAbs(dst, abs_dst);
    cv::applyColorMap(abs_dst, cameraFrame, cv::COLORMAP_PARULA);
    return;

    // Corner detection??
    /* cv::Mat harris; */
    /* cv::cornerHarris(gray, harris, 2, 3, 0.4); */
    /* cv::dilate(harris, harris, harris); // Marsha, Marsha, Marsha! */

    /* cv::cvtColor(harris, cameraFrame, cv::COLOR_GRAY2BGR); */
    /* return; */

    auto mult = 2;
    auto alpha = 2.0;
    auto beta = -100;

    /* cv::resize(gray, gray, cv::Size { static_cast<int>(_camWidth) / mult, static_cast<int>(_camHeight) / mult }, cv::INTER_CUBIC); */

    // cv::GaussianBlur(gray, gray, cv::Size {9, 9}, 2, 2);
    cv::Laplacian(gray, gray, 1, 3);

    cv::cvtColor(gray, cameraFrame, cv::COLOR_GRAY2BGR);
    cv::resize(cameraFrame, cameraFrame, cv::Size { static_cast<int>(_camWidth), static_cast<int>(_camHeight) }, cv::INTER_NEAREST);
    /* return; */

    return;
    std::vector<cv::Vec3f> circles;

    cv::HoughCircles(
      gray
      , circles
      , cv::HOUGH_GRADIENT_ALT
      , 10
      , 1 // 7
      , 300
      , 0.9
      , 1
      , 12
    );

    std::sort(circles.begin(), circles.end(), [](auto a, auto b) {
        return a[1] > b[1];
    });

    for(auto i = 0; i < circles.size(); i++) {
      auto c = circles[i];
      auto color = CV_RGB(255.0, 0.0, 0.0);

      cv::Point center(c[0], c[1]);

      if(i == 0) {
        color = CV_RGB(0.0, 255.0, 255.0);

        std::stringstream ss;
        ss << c[2];

        _cv_ft2->putText(
            cameraFrame
            , ss.str()
            , center
            , 60
            , color
            , -1 // negative thickness fills the text.
            , cv::LINE_AA
            , true);
      }
      cv::circle(cameraFrame, center, c[2], color, 2);
    }

  } catch (cv::Exception cve) {
    std::cerr << "Caught exception! " << cve.what() << std::endl;
    stop = true;
    return;
  }
}
