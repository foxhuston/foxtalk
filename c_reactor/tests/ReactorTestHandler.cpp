//
// Created by fox on 10/2/24.
//

#include <cstdint>
#include <cstddef>
#include <functional>

#include "Tuple.h"

extern "C" foxtalk::Tuple* get_query() {
    return new foxtalk::Tuple(
            foxtalk::TupleNoun::mkQuery(),
            foxtalk::TupleNoun::mkSymbol("is a"),
            foxtalk::TupleNoun::mkSymbol("husky")
    );
}

extern "C" void handle_results(foxtalk::TupleVec query_results, std::function<void(foxtalk::Tuple)> claim) {
    for(auto r : query_results) {
        std::cout << "Hello from dynamic handler (calling claim)! Handler query result: " << r << std::endl;

        claim({
            r.getSubject(),
            foxtalk::TupleNoun::mkSymbol("is"),
            foxtalk::TupleNoun::mkSymbol("super cool"),
        });

        claim({
            r.getSubject(),
            foxtalk::TupleNoun::mkSymbol("is a"),
            foxtalk::TupleNoun::mkSymbol("great puppy"),
        });
    }
}

