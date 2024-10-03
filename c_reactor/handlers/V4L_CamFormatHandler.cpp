//
// Created by fox on 10/2/24.
//

#include <iostream>
#include <cstdint>
#include <cstddef>
#include <functional>

#include "ReactorSet.h"
#include "Tuple.h"

extern "C" foxtalk::Tuple* get_query() {
    return foxtalk::Tuple::mk(
            mkQuery(),
            mkSymbol("is a"),
            mkSymbol("camera")
    );
}

extern "C" void handle_results(foxtalk::ReactorVec<const foxtalk::Tuple*>::type query_results, std::function<void(const foxtalk::Tuple*)> claim) {
    for(auto r : query_results) {
        std::cout << "Hello from V4L handler! Handler query result: " << r << std::endl;
    }
}

