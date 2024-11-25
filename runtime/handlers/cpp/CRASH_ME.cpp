#include <foxtalk_handler.hpp>

class CrashMeHandler : public Handler
{
public:
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    // claim({{
    //   {"foxtalk"},
    //   {"handlers"},
    //   {"exist"},
    //   {"at"},
    //   {"absolute"},
    //   {"path"},
    //   {"/tmp"}
    // }});
  }

  void init() override {
    claim({{
      {"foxtalk"},
      {"is"},
      {"running"}
    }});

    int *x = nullptr;
    std::cout << "I'm in danger! " << *x << std::endl;
  }
};

FOXTALK_FFI_HANDLER_REG(CrashMeHandler);