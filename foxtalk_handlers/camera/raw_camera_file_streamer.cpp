#include <foxtalk_handler.hpp>
#include <fstream>

class RawCameraFileStreamer : public Handler
{

bool c = true;
public:
  bool poll() override {
    return c;
  }
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (!c) return;
    if (queryResults.size() > 0) {
      auto q = queryResults[0];
      auto data = static_cast<uint8_t*>(q.at<void *>(8).value());
      
      auto pixel_format = q.at<uint64_t>(2).value();
      auto width = q.at<uint64_t>(4).value();
      auto height = q.at<uint64_t>(6).value();
      std::ofstream myfile;
      myfile.open ("/home/lexi/work/foxtalk/video.yuyv");
      
      
      for (auto i = 0; i <= height*width*2; i++) {
        myfile << data[i];
      }
      myfile.close();
      c = false; 
    }
  }

  void init() override {

          claim({
            {TupleNoun::query(),
            {"with pixel format"}, 
            TupleNoun::query(),
            {"with resolution width"},
            TupleNoun::query(),
            {"with resolution height"}, 
            TupleNoun::query(),
            {"has image"},
            TupleNoun::prefix(),
          }});
  }
};

FOXTALK_FFI_HANDLER_REG(RawCameraFileStreamer);