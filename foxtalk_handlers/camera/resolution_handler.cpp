#include <foxtalk_handler.hpp>

class ExampleHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {}

  void init() override {}
};

FOXTALK_FFI_HANDLER_REG(ExampleHandler);
