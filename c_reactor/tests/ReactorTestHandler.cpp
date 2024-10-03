//
// Created by fox on 10/2/24.
//

#include <iostream>
#include <functional>

#include "Tuple.h"
#include "ReactorSet.h"

extern "C" foxtalk::Tuple* get_query() {
    return foxtalk::Tuple::mk(
            foxtalk::TupleNoun::mkQuery(),
            foxtalk::TupleNoun::mkSymbol("is a"),
            foxtalk::TupleNoun::mkSymbol("husky")
    );
}

extern "C" void handle_results(foxtalk::ReactorVec<const foxtalk::Tuple*>::type query_results, std::function<void(const foxtalk::Tuple *)> claim) {
    for(auto r : query_results) {
        std::cout << "Hello from dynamic handler (calling claim)! Handler query result: " << r << std::endl;

        claim(foxtalk::Tuple::mk(
            r->getSubject(),
            foxtalk::TupleNoun::mkSymbol("is"),
            foxtalk::TupleNoun::mkSymbol("super cool")
        ));

        claim(foxtalk::Tuple::mk(
            r->getSubject(),
            foxtalk::TupleNoun::mkSymbol("is an"),
            foxtalk::TupleNoun::mkSymbol("awesome puppy")
        ));
    }
}

