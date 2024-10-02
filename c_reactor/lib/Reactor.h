//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "gc_cpp.h"
#include "gc_allocator.h"

#include "boost/functional/hash.hpp"

#include "Db.h"
#include "Tuple.h"
#include "Handler.h"

#include "ReactorCache.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor : gc {
    private:
        Db db {};
        std::vector<const Handler *> handlers { };
//        std::unordered_map<Tuple, Tuple, traceable_allocator<Tuple>> tuple_provenance {};

        // All of these should at least be hashsets...
//        std::vector<std::pair<Tuple, const Handler *>, traceable_allocator<std::pair<Tuple, const Handler *>>> tuple_handler_activations {};
//        std::vector<std::pair<const Handler *, Tuple>, traceable_allocator<std::pair<const Handler *, Tuple>>> handler_provenance {};

        ReactorCache<std::pair<Tuple, Tuple>>::Set tuple_provenance {};
    public:
        void claim(const Tuple& tuple);
        void remove(const Tuple& tuple);
        void add_handler(const Handler* handler);
        void tick();

        // TODO: FOR TESTING ONLY!!!
        const Db& get_db() const {
            return db;
        }
    };

} // foxtalk

#endif //REACTOR_REACTOR_H
