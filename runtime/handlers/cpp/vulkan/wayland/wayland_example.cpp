#include <foxtalk_handler.hpp>

class WaylandExampleHandler : public Handler
{
public:
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    // claim({{
    //   {"WaylandExampleHandler"},
    //   {"wishes for"},
    //   {"a wayland surface"},
    //   {"at x"},
    //   {500},
    //   {"at y"},
    //   {50},
    //   {"with width"},
    //   {250},
    //   {"with height"},
    //   {150},
    //   {"with title"},
    //   {"Hello from example wayland handler"},
    // }});
    // claim({{
    //   {"WaylandExampleHandler"},
    //   {"wishes for"},
    //   {"a wayland surface"},
    //   {"at x"},
    //   {300},
    //   {"at y"},
    //   {0},
    //   {"with width"},
    //   {250},
    //   {"with height"},
    //   {150},
    //   {"with title"},
    //   {"Hello from example wayland handler"},
    // }});
    // claim({{
    //   {"WaylandExampleHandler"},
    //   {"wishes for"},
    //   {"a wayland surface"},
    //   {"at x"},
    //   {600},
    //   {"at y"},
    //   {150},
    //   {"with width"},
    //   {25},
    //   {"with height"},
    //   {25},
    //   {"with title"},
    //   {"Hello from example wayland handler"},
    // }});
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(WaylandExampleHandler);