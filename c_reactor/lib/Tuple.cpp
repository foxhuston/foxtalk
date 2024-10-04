//
// Created by fox on 10/2/24.
//

#include "Tuple.h"

#include "boost/container_hash/hash.hpp"

//#include "gc.h"
//#include "gc_allocator.h"

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

    std::size_t hash_value(const TupleNoun *t) {
        return hash_value(*t);
    }

    std::size_t hash_value(const Tuple *t) {
        return hash_value(*t);
    }

    Tuple *Tuple::mk(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object) {
        auto a = new Tuple{ subject, predicate, object };
//        std::cout << "New Tuple " << *a << " @ " << a << std::endl;
        return a;
//        auto t = (Tuple*) GC_debug_malloc(sizeof(Tuple), "Tuple", 0);
//        t->subject = subject;
//        t->predicate = predicate;
//        t->object = object;
//
//        return t;
    }
}