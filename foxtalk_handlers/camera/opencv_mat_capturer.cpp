// pkg-config: opencv4
// cppstd: 23
#include <opencv2/opencv.hpp>
#include <iostream>

#include <foxtalk_handler.hpp>

class OpenCvMatCapturerHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() != 1) {
      return;
    }
    auto q = queryResults[0];
    cv::VideoCapture* cap = static_cast<cv::VideoCapture*>(q.at<void *>(0).value());

    cv::Mat frame;
    *cap >> frame; 
    
    std::cout << "Testing\n";
    
  }
    

  void init() override {

      claim({{
          {TupleNoun::query()}, 
          {"is an"},
          {"opencv4 video capture object"},
      }});
  }
};

FOXTALK_FFI_HANDLER_REG(OpenCvMatCapturerHandler);