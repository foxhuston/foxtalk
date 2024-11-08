#include <foxtalk_handler.hpp>

class CameraStateWatchingHandler : public Handler
{

public:
  bool poll() override {
    return false;
  }
protected:
  void handle(const std::vector<Tuple> &queryResults) override {}

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }

};

FOXTALK_FFI_HANDLER_REG(CameraStateWatchingHandler);