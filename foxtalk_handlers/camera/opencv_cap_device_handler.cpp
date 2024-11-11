// pkg-config: opencv4
// cppstd: 23
#include <opencv2/opencv.hpp>
#include <iostream>

#include <foxtalk_handler.hpp>

class OpenCvCaptureDeviceHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() != 1) {
      return;
    }
    auto q = queryResults[0];
    auto camera = q.at<std::string>(2).value();
    std::cout << camera << std::endl;
    cv::VideoCapture cap(camera);
    
    claim({{{(void *)&cap}, {"is an"}, {"opencv4 video capture object"}}});
  }

  void init() override {

      claim({{
          {"chosen foxtalk camera"},
          {"is"},
          {TupleNoun::query()}, 
          {"with pixel format"},
          {TupleNoun::query()},
          {"with resolution width"},
          {TupleNoun::query()},
          {"with resolution height"},
          {TupleNoun::query()},
          {"with fps"},
          {TupleNoun::query()},}});
  }
};

FOXTALK_FFI_HANDLER_REG(OpenCvCaptureDeviceHandler);