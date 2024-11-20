#include <foxtalk_handler.hpp>

class ExampleHandler : public Handler {

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    claim({{{"illumination"},
            {"rectangle"},
            {"at"},
            {"x"},
            {10ul},
            {"y"},
            {10ul}, 
            {"width"},
            {200ul}, 
            {"height"},
            {200ul}
    }});
  }

  void init() override { claim({{{"foxtalk"}, {"is"}, {"running"}}}); }
};

FOXTALK_FFI_HANDLER_REG(ExampleHandler);