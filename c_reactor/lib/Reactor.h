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

#include "ReactorSet.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor : gc {
    private:
        Db db {};
        std::vector<const Handler *> handlers { };

        ReactorMap<ReactorSet<Tuple>::type, ReactorSet<const Handler*>::type>::type tuples_triggered_handler {};
        ReactorMap<Tuple, ReactorSet<Tuple>::type>::type tuple_provenance {};

        void tuple_prov_insert(Tuple from, Tuple to);

        void tuples_triggered_handler_insert(ReactorSet<Tuple>::type from, const Handler *to);
        bool did_tuples_trigger_handler(ReactorSet<Tuple>::type from, const Handler *to);
    public:
        void claim(const Tuple& tuple);
        void remove(const Tuple& tuple);
        void add_handler(const Handler* handler);
        void tick();

        // TODO: FOR TESTING ONLY!!!
        const Db& get_db() const {
            return db;
        }

        ReactorSet<Tuple>::type query(Tuple q);

        ReactorSet<foxtalk::Tuple>::type query(Tuple *q);
    };

} // foxtalk

#endif //REACTOR_REACTOR_H
