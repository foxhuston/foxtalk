#include <iostream>

#include "DynamicCompilingHandler.h"
#include "Reactor.h"
#include "Tuple.h"

int main() {
  foxtalk::Reactor reactor{};
  foxtalk::DynamicCompilingHandler d{&reactor, "handlers/"};

  reactor.claim(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));
  reactor.remove(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));
  reactor.claim(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));
  reactor.claim(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));
  reactor.remove(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));
  reactor.remove(mkSymbol("/dev/video0"), mkSymbol("is a"), mkSymbol("camera"));

  while (true) {
    reactor.tick();
    //        std::cout << "Tick!" << std::endl;
    //        for(auto t : reactor.get_db().get_tuples()) {
    //            std::cout << "  In DB: " << t << std::endl;
    //        }
    sleep(1);
  }

  return 0;
}
