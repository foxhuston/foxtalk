#include <iostream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/freetype.hpp>

extern "C" void process_image(
    cv::Mat& cameraFrame
    , uint32_t _camWidth
    , uint32_t _camHeight
    , cv::freetype::FreeType2* _cv_ft2
) {
  // view from tabletop cam is inverted
  auto rot_mat = cv::getRotationMatrix2D(
      { static_cast<float>(_camWidth) / 2.0f, static_cast<float>(_camHeight) / 2.0f }
      , 180.0
      , 1.0);

  cv::warpAffine(cameraFrame, cameraFrame, rot_mat, cameraFrame.size());

  cv::Mat gray;
  cv::cvtColor(cameraFrame, gray, cv::COLOR_BGR2GRAY);

  std::vector<cv::Vec3f> circles;
  cv::HoughCircles(
    gray
    , circles
    , cv::HOUGH_GRADIENT_ALT
    , 10
    , 2
    , 1000
    , 0.9
    , 5
    , 20
  );

  /* std::sort(circles.begin(), circles.end(), [](autoa); */

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

      /* cv::putText( */
      /*   cameraFrame */
      /*   , ss.str() */
      /*   , center */
      /*   , cv::FONT_HERSHEY_SIMPLEX */
      /*   , 2.0 */
      /*   , color */
      /* ); */
    }
    cv::circle(cameraFrame, center, c[2], color, 2);
  }
}
