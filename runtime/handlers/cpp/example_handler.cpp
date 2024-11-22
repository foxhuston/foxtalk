#include <foxtalk_handler.hpp>

class ExampleHandler : public Handler
{
public:
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    claim({{
      {"foxtalk"},
      {"handlers"},
      {"exist"},
      {"at"},
      {"absolute"},
      {"path"},
      {"/tmp"}
    }});
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(ExampleHandler);