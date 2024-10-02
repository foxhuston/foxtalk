//
// Created by fox on 10/2/24.
//

#include "Tuple.h"

namespace foxtalk {
    std::size_t hash_value(const TupleNoun &t) {
        return t.hash();
    }

    std::size_t hash_value(const Tuple &t) {
        size_t seed = 0;
        boost::hash_combine(seed, *t.getSubject());
        boost::hash_combine(seed, *t.getPredicate());
        boost::hash_combine(seed, *t.getObject());
        return seed;
    }
}