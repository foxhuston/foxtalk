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

    while (true) {
        // Capture a new frame
        *cap >> frame; // or cap.read(frame);
        if (frame.empty()) {
            std::cerr << "Error: Could not grab a frame." << std::endl;
            break;
        }

        // Display the frame
        cv::imshow("Camera Feed", frame);

        // Wait for 30 ms and check if the user pressed 'q' to exit
        if (cv::waitKey(30) >= 0) break;
    }
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