#include "opencv2/core/base.hpp"
#include "opencv2/core/hal/interface.h"
#include "opencv2/core/matx.hpp"
#include "opencv2/core/types.hpp"
#include <algorithm>
#include <numbers>
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

#include "ReactorSet.h"
#include "Tuple.h"


///// FOXTALK API //////////////////////////////////////////////////////////////

extern "C" foxtalk::Tuple* get_query() {
  return foxtalk::Tuple::mk(
      mkQuery(),
      mkSymbol("is a"),
      mkSymbol("camera frame")
  );
}

extern "C" void handle_results(foxtalk::ReactorVec<const foxtalk::Tuple*>::type query_results, std::function<void(const foxtalk::Tuple*)> claim) {
  for(auto r : query_results) {
//    std::cout << "Hello from CvDotSandbox handler! Handler query result: " << *r << std::endl;

    if(r->getSubject()->is_cptr()) {
      cv::Mat *cameraMat = (cv::Mat *)r->getSubject()->data.cptr.data;

      auto outputMat = new cv::Mat(*cameraMat);
      *outputMat *= 0.5;

      claim(foxtalk::Tuple::mk(
          mkPtr(outputMat, r->getSubject()->data.cptr.free_fn),
          mkSymbol("is a"),
          mkSymbol("output layer")
      ));
    }
  }
}



////////////////////////////////////////////////////////////////////////////////

constexpr int COLOR_THRESHOLD = 50;

///// Marker Cluster ///////////////////////////////////////////////////////////

/* New clustering idea:
 *
 * - Partition incoming points into possible groups:
 *   - A "possible group" is a collection of points where
 *     each point is within the allowableDistance of any
 *     ONE other point. So it kind of globs more and more as it grows.
 *   - For each set of points, find a candidate triangle that intersects
 *     five points total.
 *   - Remove those five points from the pool. If there are >5 points left,
 *     find a new candidate triangle.
 *     - Otherwise, repartition the points.
 * - At the end, there may (probably will) be points that have not made it into
 *   any triangles.
 */

class TriLine {
public:
  cv::Point2f a, b, normal;
  float length;

  float dist(cv::Point2f pt) {
    return std::abs(normal.dot(a - pt));
  }

  TriLine()
    : a {0.0f, 0.0f}, b {0.0f, 0.0f}, normal {0, 0}, length { 0 }
  {
  }

  TriLine(cv::Point2f a, cv::Point2f b)
      : a { a }, b { b }
  {
    length = cv::norm(a - b);
    auto n = (a - b) / length;
    normal = { n.y, -n.x };
  }

  void draw(const cv::Mat& cameraFrame, const cv::Scalar color = CV_RGB(255, 0, 0)) {
    constexpr int ray_length = 10000;

    // Draw line
    cv::line(
      cameraFrame
      , a
      , b
      , color
      , 2
    );

    // Draw Normal
    cv::line(
      cameraFrame
      , a
      , a + 100 * normal
      , CV_RGB(255, 255, 255)
      , 2
      );

    cv::circle(cameraFrame, { static_cast<int>(a.x), static_cast<int>(a.y) }, 5, CV_RGB(255, 255, 255), -1);
  }

};

////////////////////////////////////////////////////////////////////////////////

static bool stop = false;
static cv::Ptr<cv::SimpleBlobDetector> blob;

static std::array<cv::Vec3b, 2> colorsBgr = {{
  { 121, 146, 44  },   // Green
  { 88,  56,  199 }    // Red
}};

static std::array<cv::Vec3f, 2> colorsHsv = {{
  { 85.0f, 178.0f, 146.0f   },   // Green
  { 171.0f, 183.0f, 199.0f   }    // Red
}};

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

void lollipop(cv::Mat& output, cv::Point2f from, cv::Point2f to, cv::Scalar color = CV_RGB(255, 255, 255), float radius = 20) {
  auto length = cv::norm(to - from);
  auto dir = (to - from) / length;

  cv::line(output, from, from + dir * (length - radius), color, 2);
  cv::circle(output, to, radius, color, 2);
}

extern "C" void process_image(
    cv::Mat& cameraFrame
    , cv::Mat& outputFrame
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

    // Blob Detection
    std::vector<cv::KeyPoint> keypoints;
    blob->detect(gray, keypoints);

    std::vector<cv::KeyPoint> greenKeypoints;
    std::vector<cv::KeyPoint> redKeypoints;

    for(auto kp : keypoints) {
      cv::Vec3b blobBgr = cameraFrame.at<cv::Vec3b>(kp.pt);
      cv::Vec3f blobHsv = hsv.at<cv::Vec3f>(kp.pt);

      uint32_t min_n = std::numeric_limits<uint32_t>::max();
      cv::Vec3f minHsv = { 127, 127, 127 };
      cv::Vec3f minBgr = { 127, 127, 127 };

      uint8_t minIdx;

      for(int i = 0; i < colorsBgr.size(); i++) {
        cv::Vec3b bgr = colorsBgr[i];
        cv::Vec3f hsv = colorsHsv[i];

        // Ignore brightness.
        /* hsv[2] = 0; */
        /* blobHsv[2] = 0; */

        float n = cv::norm(hsv - blobHsv, cv::NORM_L2);

        if(n < min_n) {
          min_n = n;
          minHsv = hsv;
          minBgr = bgr;
          minIdx = i;
        }
      }

      /* if(min_n < COLOR_THRESHOLD) { */
      if(min_n < COLOR_THRESHOLD) {
        /* std::cout << "Assigned "; */
        /* printColor(colorsBgr[minIdx], colorsHsv[minIdx]); */
        /* std::cout << " for detected color "; */
        /* printColor(blobBgr, blobHsv); */
        /* std::cout << std::endl; */
        if(minIdx == 0) {
          greenKeypoints.push_back(kp);
        } else {
          redKeypoints.push_back(kp);
        }
      }
      /* else { */
      /*   std::cout << "NO ASSIGNMENT for detected color "; */
      /*   printColor(blobBgr, blobHsv); */
      /*   std::cout << std::endl; */
      /* } */
    }

    ///// DRAW ANY FOUND POINTS //////////////////////////////////////////////

    for(auto& kp : greenKeypoints) {
      cv::circle(
          outputFrame
          , kp.pt
          , 20
          , CV_RGB(0, 255, 0)
          , 2
      );
    }

    for(auto& kp : redKeypoints) {
      // std::cout << "Drawing a red keypoint!" << std::endl;
      cv::circle(
          outputFrame
          , kp.pt
          , 20
          , CV_RGB(255, 0, 0)
          , 2
      );
    }

    ///// GEOMETRY DEMOS /////////////////////////////////////////////////////


    // The two green points can be thought of as a step in like... "For every
    // pair of points in a cluster..."
    if(greenKeypoints.size() == 2) {
      TriLine line { greenKeypoints[0].pt, greenKeypoints[1].pt };
      line.draw(cameraFrame, CV_RGB(0, 255, 0));

      // We kind of already know the four places a corner could be
      // because we know that the dots form right triangles...

      ///// EXPECTED CORNERS /////
      auto expectedA1 = line.a + line.normal * line.length;
      auto expectedA2 = line.a - line.normal * line.length;
      auto expectedB1 = line.b + line.normal * line.length;
      auto expectedB2 = line.b - line.normal * line.length;

      lollipop(cameraFrame, line.a, expectedA1);
      lollipop(cameraFrame, line.a, expectedA2);
      lollipop(cameraFrame, line.b, expectedB1);
      lollipop(cameraFrame, line.b, expectedB2);

      ///// RED POINTS /////
      std::vector<cv::Point2f> expectedPoints { expectedA1, expectedA2, expectedB1, expectedB2 };

      float minDist = std::numeric_limits<float>::max();
      cv::Point2f minRedPt;
      for(auto& redPoint : redKeypoints) {
        for(auto& expectedPt : expectedPoints) {
          auto dist = cv::norm(redPoint.pt - expectedPt);
          if(dist < minDist) {
            minDist = dist;
            minRedPt = redPoint.pt;
          }
        }
      }

      cv::circle(
        cameraFrame
        , minRedPt
        , 30
        , CV_RGB(255, 255, 0)
        , 2
      );

      ///// OLD VIS /////
      for(auto& redPoint : redKeypoints) {
        auto dist = line.dist(redPoint.pt);
        /* std::cout << "What " << dist << std::endl; */

        // Above or below???
        auto n = redPoint.pt.y < line.a.y
          ? line.normal : -line.normal;

        auto endpoint = redPoint.pt + (-n) * dist;

        // Draw line to midline
        cv::line(
          cameraFrame
          , redPoint.pt
          , endpoint
          , CV_RGB(255, 0, 0)
          , 2
        );

        // Show the angle between the point & the line normal...
        auto normDirA =
          (redPoint.pt - line.a) / cv::norm(redPoint.pt - line.a);

        auto alignmentA = (1 + normDirA.dot(n)) / 2;

        auto normDirB =
          (redPoint.pt - line.b) / cv::norm(redPoint.pt - line.b);

        auto alignmentB = (1 + normDirB.dot(n)) / 2;

        if(alignmentA > alignmentB) {
          auto col = static_cast<int>(alignmentA * 255);
          cv::line(
            cameraFrame
            , redPoint.pt
            , line.a
            , CV_RGB(0, col, col)
            , 2
          );
        } else {
          auto col = static_cast<int>(alignmentB * 255);
          cv::line(
            cameraFrame
            , redPoint.pt
            , line.b
            , CV_RGB(0, col, col)
            , 2
          );
        }
      }
    }

    cv::rectangle(outputFrame, {0, 0, 1920, 1080}, CV_RGB(0, 255, 255), -1);

    ///// DOT CLUSTER FINDER /////////////////////////////////////////////////


    /* cv::drawKeypoints(cameraFrame, keypoints, cameraFrame); */


  } catch (cv::Exception cve) {
    std::cerr << "Caught exception! " << cve.what() << std::endl;
    stop = true;
    return;
  }
}
