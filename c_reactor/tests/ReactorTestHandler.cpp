//
// Created by fox on 10/2/24.
//

#include <cstdint>
#include <cstddef>

#include "Tuple.h"

extern "C" foxtalk::Tuple* get_query() {
    return new foxtalk::Tuple(
            foxtalk::TupleNoun::mkQuery(),
            foxtalk::TupleNoun::mkSymbol("is a"),
            foxtalk::TupleNoun::mkSymbol("husky")
    );
}

extern "C" void handle_results(foxtalk::TupleVec query_results) {
    for(auto& r : query_results) {
        std::cout << "Hello from dynamic handler! Handler query result: " << r << std::endl;
    }
}

