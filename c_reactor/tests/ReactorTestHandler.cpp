//
// Created by fox on 10/2/24.
//

#include <functional>
#include <iostream>

#include "ReactorSet.h"
#include "Tuple.h"

extern "C" foxtalk::Tuple *get_query() {
  return foxtalk::Tuple::mk(mkQuery(), mkSymbol("is a"), mkSymbol("husky"));
}

extern "C" void
handle_results(foxtalk::ReactorVec<const foxtalk::Tuple *>::type query_results,
               std::function<void(const foxtalk::Tuple *)> claim) {
  for (auto r : query_results) {
    std::cout
        << "Hello from dynamic handler (calling claim)! Handler query result: "
        << r << std::endl;

    claim(foxtalk::Tuple::mk(r->getSubject(), mkSymbol("is"),
                             mkSymbol("super cool")));

    claim(foxtalk::Tuple::mk(r->getSubject(), mkSymbol("is an"),
                             mkSymbol("awesome puppy")));
  }
}
