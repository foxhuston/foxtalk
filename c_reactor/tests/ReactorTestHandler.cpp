//
// Created by fox on 10/2/24.
//

#include <cstdint>
#include <cstddef>

#include "Tuple.h"

foxtalk::Tuple* get_query() {
    return new foxtalk::Tuple(
            foxtalk::TupleNoun::mkQuery(),
            foxtalk::TupleNoun::mkSymbol("is a"),
            foxtalk::TupleNoun::mkSymbol("husky")
    );
}

void handle_results(foxtalk::TupleVec query_results) {
    for(auto& r : query_results) {
        std::cout << "Handler query result: " << r << std::endl;
    }
}

