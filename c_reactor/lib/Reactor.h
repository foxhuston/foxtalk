//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include <mutex>
#include <queue>

#include "boost/functional/hash.hpp"

#include "Db.h"
#include "Tuple.h"
#include "Handler.h"

#include "ReactorSet.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor {
    private:
        std::mutex handlerMutex{};
        std::mutex tupleMutex{};

        Db db{};
        ReactorSet<const Handler *>::type handlers{};

        ReactorMap<ReactorSet<const Tuple *>::type, ReactorSet<const Handler *>::type>::type tuples_triggered_handler{};
        ReactorMap<const Tuple *, ReactorSet<const Tuple *>::type>::type tuple_provenance{};
        ReactorMap<const Handler *, ReactorSet<const Tuple *>::type>::type tuple_handler_provenance{};

        void tuples_triggered_handler_insert(ReactorSet<const Tuple *>::type from, const Handler *to);

        bool did_tuples_trigger_handler(ReactorSet<const Tuple *>::type from, const Handler *to);

    public:
        void claim(const Tuple *tuple);

        void claim(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object);

        void remove(const Tuple *tuple);

        void remove(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object);

        void add_handler(const Handler *handler);

        void remove_handler(const Handler *handler);

        void tick();

        // TODO: FOR TESTING ONLY!!!
        const Db &get_db() const {
            return db;
        }

        ReactorSet<const Tuple *>::type query(const Tuple *q);

        ReactorSet<const Tuple *>::type
        query(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object);

        void remove(std::queue<const Tuple *> workQueue);

        void remove(ReactorSet<const Tuple *>::type tuples);
    };

} // foxtalk

#endif //REACTOR_REACTOR_H
