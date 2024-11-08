// pkg-config: opencv4
// cppstd: 23
#include <foxtalk_handler.hpp>
#include <opencv2/opencv.hpp>
#include <ostream>

class NewCamHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    cv::VideoCapture cap(0); // 0 is the default camera index 



    // Check if the webcam is opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the webcam." << std::endl;
    }
    else {
      std::cout << "cap opened!" << std::endl; 
    }
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(NewCamHandler);