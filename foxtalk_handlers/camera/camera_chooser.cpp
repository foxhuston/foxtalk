#include <foxtalk_handler.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

class CameraChooser : public Handler
{
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.empty()) {
      return;
    }
    auto best_score = 0.0;
    auto best_camera = ""s;
    auto best_pixel_format = 0ul;
    auto best_width = 0ul;
    auto best_height = 0ul;
    auto best_fps = 0.0;
    for (auto q: queryResults) {
      auto camera = q.at<std::string>(0).value();
      auto pixel_format = q.at<uint64_t>(2).value();
      auto width = q.at<uint64_t>(4).value();
      auto height = q.at<uint64_t>(6).value();
      auto fps = q.at<double_t>(8).value();

      auto this_camera_score = (width/10) * (height/10) * fps;
      if (this_camera_score > best_score) {
        best_score = this_camera_score;
        best_camera = camera; 
        best_pixel_format = pixel_format;
        best_width = width;
        best_height = height;
        best_fps = fps;
      } 
    }
    std::cout << "chosen: " << best_camera << " with score " << best_score << std::endl;
    claim({{
        {"chosen foxtalk camera"},
        {"is"},
        {best_camera}, 
        {"with pixel format"},
        {best_pixel_format},
        {"with resolution width"},
        {best_width},
        {"with resolution height"},
        {best_height},
        {"with fps"},
        {best_fps},}});
  }
  void init() override
  {
    //Library@0x7504780018c0
    claim({{
      TupleNoun::query(),
      {"has pixel format"},
      {TupleNoun::query()},
      {"with resolution width"},
      {TupleNoun::query()},
      {"with resolution height"},
      {TupleNoun::query()},
      {"with fps"},
      {TupleNoun::query()}
      }});
  }

};

FOXTALK_FFI_HANDLER_REG(CameraChooser);
