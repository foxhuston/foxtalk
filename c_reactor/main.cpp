#include <iostream>

#include "Tuple.h"
#include "Reactor.h"
#include "DynamicHandler.h"

int main() {
    foxtalk::Reactor reactor {};
    foxtalk::DynamicHandler d { &reactor, "../tests/libreactor_test_handler.so" };

    reactor.claim({
                          foxtalk::TupleNoun::mkSymbol("lexi"),
                          foxtalk::TupleNoun::mkSymbol("is a"),
                          foxtalk::TupleNoun::mkSymbol("husky")
                  });

    while(true) {
        reactor.tick();
        std::cout << "Tick!" << std::endl;
        for(auto t : reactor.get_db().get_tuples()) {
            std::cout << "  In DB: " << t << std::endl;
        }
        sleep(5);
    }

    return 0;
}
