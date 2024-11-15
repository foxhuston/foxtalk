// pkg-config: opencv4
// cppstd: 23

// #include "opencv2/videoio.hpp"
#include <cstdint>
// #include <opencv2/core.hpp>
#include <iostream>

//Library@0x76db80001880

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
    std::cout << "Testing 3!" << camera << std::endl;


    // int apiID = cv::CAP_V4L2;
    // auto *vc = new cv::VideoCapture {};

    // auto width = q.at<uint64_t>(6).value();
    // auto height = q.at<uint64_t>(8).value();
    // auto fps = q.at<double>(10).value();


    // vc->set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, width);
    // vc->set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, height);
    // vc->set(cv::VideoCaptureProperties::CAP_PROP_FPS, fps);
    // vc->open(camera, apiID);
    
    // claim({{{(void *)vc}, {"is an"}, {"opencv4 video capture object"}}});
    
    // ///// Run the frame...
    // cv::Mat cameraFrame;

    // vc->read(cameraFrame);
    // // check if we succeeded
    // if (cameraFrame.empty()) {
    //   throw std::runtime_error("ERROR! blank frame grabbed");
      
    // }

    // cv::Size s = cameraFrame.size();
    // auto rows = s.height;
    // auto cols = s.width;
    // std::cout << "Num channels in image: " << cameraFrame.channels() << std::endl;
    // std::cout << "Rows: " << rows << "... cols: " << rows << std::endl;
    // std::cout << "About to save image..." << std::endl;

    // // std::cout << "Testing @ save image..." << std::endl;

    // cv::imwrite("/home/lexi/work/foxtalk/test.jpg", cameraFrame);
 
    // std::cout << "Saved image... 3" << std::endl;

    // auto rot_mat = cv::getRotationMatrix2D(
    //     {static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f}, 180.0, 1.0);

    // cv::warpAffine(cameraFrame, cameraFrame, rot_mat, cameraFrame.size());


    

    // Display the image in a window
    // cv::imshow("Display Image", cameraFrame);

    // std::cout << "Showed image, waiting..." << std::endl;
    // Wait for a key press indefinitely
    // cv::waitKey(0);
    // std::cout << "Caught key?" << std::endl;
  
  }

  void free_tuple(const Tuple &t) override {
    std::cout << "Freeing memory for opencv video capture" << std::endl;
    std::cout << t << std::endl; 
  
    // if (auto ptr = t.at<void *>(0)) {
    //   delete static_cast<cv::VideoCapture*>(ptr.value());
    // }
  } 

  void init() override {

    std::cout << "In init for cap device handler so! FOUR" << std::endl;

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
      {TupleNoun::query()},
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(OpenCvCaptureDeviceHandler);