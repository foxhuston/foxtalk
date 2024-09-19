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


constexpr int COLOR_THRESHOLD = 50;

///// Marker Cluster ///////////////////////////////////////////////////////////
struct Marker {
  uint8_t numericValue;
  float rad;
  cv::Point2f pos;

  Marker(uint8_t numericValue, float rad, cv::Point2f pos)
      : numericValue(numericValue), rad(rad), pos(pos) {}
};

struct MarkerCluster {
  std::vector<Marker> markers { };

  MarkerCluster(Marker firstMarker)
  : representativeMarker(firstMarker)
  {
    markers.push_back(firstMarker);

    // I desparately need to write some notes, here.
    // Basically this circumscribes a right triangle; we know the
    // dots are all the same size for a given marker, and we know the
    // distances between them.
    allowableDistance = std::sqrt(2.0f) * 6 * firstMarker.rad;
  }

  bool tryAddMarker(Marker newMarker) {
    if(markers.size() == 5) return false;

    cv::Point2f dd = representativeMarker.pos - newMarker.pos;
    auto dist = cv::norm(dd);

    if(dist <= allowableDistance) {
      markers.push_back(newMarker);
      return true;
    }

    return false;
  }

  std::array<cv::Point2f, 3> getBoundingTriangle(const cv::Mat& cameraFrame) {
    auto min_x = std::min_element(markers.begin(), markers.end(), [](auto m1, auto m2) {
        return m1.pos.x < m2.pos.x;
    });

    auto max_x = std::max_element(markers.begin(), markers.end(), [](auto m1, auto m2) {
        return m1.pos.x < m2.pos.x;
    });

    auto final_pt = std::max_element(markers.begin(), markers.end(), [&min_x, &max_x](auto m1, auto m2) {
        return
          (cv::norm(min_x->pos - m1.pos) + cv::norm(max_x->pos - m1.pos))
           < (cv::norm(min_x->pos - m2.pos) + cv::norm(max_x->pos - m2.pos));
    });

    // MIN X: WHITE
    cv::circle(
        cameraFrame
        , min_x->pos
        , 20
        , CV_RGB(255, 255, 255)
        , 2);

    // MAX X: TEAL
    cv::circle(
        cameraFrame
        , max_x->pos
        , 20
        , CV_RGB(0, 255, 255)
        , 2);

    // FINAL PT: PURPLE
    cv::circle(
        cameraFrame
        , final_pt->pos
        , 20
        , CV_RGB(255, 0, 255)
        , 2);


    return { min_x->pos, max_x->pos, final_pt->pos };
  }

  cv::Rect getBoundingBox() {
    float x1 = 10000, y1 = 10000, x2, y2;
    for(auto m : markers) {
      x1 = std::min(m.pos.x, x1);
      x2 = std::max(m.pos.x, x2);
      y1 = std::min(m.pos.y, y1);
      y2 = std::max(m.pos.y, y2);
    }

    return cv::Rect {
        static_cast<int>(x1)
      , static_cast<int>(y1)
      , std::max(5, static_cast<int>(x2 - x1))
      , std::max(5, static_cast<int>(y2 - y1))
    };
  }

  private:
    Marker representativeMarker;
    float allowableDistance;
};

void addMarkerToClusters(std::vector<MarkerCluster>& clusters, Marker&& newMarker) {
  for(auto& cluster : clusters) {
    if(cluster.tryAddMarker(newMarker)) {
      return;
    }
  }

  clusters.push_back(MarkerCluster(newMarker));
}


////////////////////////////////////////////////////////////////////////////////

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

    // Blob Detection
    std::vector<cv::KeyPoint> keypoints;
    blob->detect(gray, keypoints);

    std::vector<std::pair<uint8_t, cv::KeyPoint>> filteredKeypoints;
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
        hsv[2] = 0;
        blobHsv[2] = 0;

        float n = cv::norm(hsv - blobHsv, cv::NORM_L2);
        // Hue Only?
        /* float n = std::abs(hsv[0] - blobHsv[0]); */

        if(n < min_n) {
          min_n = n;
          minHsv = hsv;
          minBgr = bgr;
          minIdx = i;
        }
      }

      if(min_n < COLOR_THRESHOLD) {
        filteredKeypoints.push_back({ minIdx, kp });
      }
    }

    ///// DOT CLUSTER FINDER /////////////////////////////////////////////////
    std::vector<MarkerCluster> clusters { };
    for(auto [index, keypoint] : filteredKeypoints) {
      addMarkerToClusters(clusters, Marker { index, keypoint.size / 2, keypoint.pt });
    }

    for(auto cluster : clusters) {
      if(cluster.markers.size() == 5) {
        auto corners = cluster.getBoundingTriangle(cameraFrame);
        cv::line(cameraFrame, corners[0], corners[1], CV_RGB(255, 0, 0), 2);
        cv::line(cameraFrame, corners[1], corners[2], CV_RGB(255, 0, 0), 2);
        cv::line(cameraFrame, corners[2], corners[0], CV_RGB(255, 0, 0), 2);
      }
    }


    ///// DRAW FILTERED KEYPOINTS ////////////////////////////////////////////
    for(auto cluster : clusters) {
      // Found a valid corner!
      if(cluster.markers.size() == 5) {
        for(auto& marker : cluster.markers) {
          auto color = colorsBgr[marker.numericValue];

          cv::circle(
            cameraFrame
            , marker.pos
            , 20
            , color
            , 5
          );
        };
      }
    }

    // Draw text as a separate layer.
    for(auto cluster : clusters) {
      // Found a valid corner!
      if(cluster.markers.size() == 5) {
        for(auto marker : cluster.markers) {
          auto color = colorsBgr[marker.numericValue];

          std::stringstream ss;
          ss << (uint32_t)marker.numericValue;

          cv::Point2f offsetPt {
            marker.pos.x - 7,
            marker.pos.y + 10
          };

          _cv_ft2->putText(
              cameraFrame
              , ss.str()
              , offsetPt
              , 30
              , CV_RGB(255, 255, 255)
              , -1 // negative thickness fills the text.
              , cv::LINE_AA
              , true);

          };
      }
    }

    // Draw Reticle
    cv::line(
        cameraFrame
        , { cameraFrame.cols / 2, 0 }
        , { cameraFrame.cols / 2, cameraFrame.rows }
        , CV_RGB(255, 255, 255)
        , 1
    );

    cv::line(
        cameraFrame
        , { 0, cameraFrame.rows / 2 }
        , { cameraFrame.cols, cameraFrame.rows / 2 }
        , CV_RGB(255, 255, 255)
        , 1
    );


    /* cv::drawKeypoints(cameraFrame, keypoints, cameraFrame); */


  } catch (cv::Exception cve) {
    std::cerr << "Caught exception! " << cve.what() << std::endl;
    stop = true;
    return;
  }
}
