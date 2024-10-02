//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "gc_cpp.h"
#include "gc_allocator.h"

#include "Db.h"
#include "Tuple.h"
#include "Handler.h"

#include <unordered_map>

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor : gc {
    private:
        Db db {};
        std::vector<const Handler *> handlers { };
//        std::unordered_map<Tuple, Tuple, traceable_allocator<Tuple>> tuple_provenance {};
        std::vector<std::pair<Tuple, Tuple>, traceable_allocator<std::pair<Tuple, Tuple>>> tuple_provenance {};

    public:
        void claim(const Tuple& tuple);
        void add_handler(const Handler* handler);
        void tick();

        // TODO: FOR TESTING ONLY!!!
        const Db& get_db() const {
            return db;
        }
    };

} // foxtalk

#endif //REACTOR_REACTOR_H
