//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "gc_cpp.h"

#include "Db.h"
#include "Tuple.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    typedef Tuple*(*GetQuery)(void);
    typedef void(*HandleResults)(size_t result_count, const Tuple* result_tuples);

    struct Handler : gc {
        GetQuery get_query;
        HandleResults handle_results;

        Handler(Tuple* (*getQuery)(void), void (*handleResults)(size_t, const Tuple *))
            : get_query { getQuery }, handle_results { handleResults } {
            assert(get_query != nullptr);
            assert(handleResults != nullptr);
        };
    };

    class Reactor : gc {
    private:
        Db db {};
        std::vector<Handler, traceable_allocator<Handler>> handlers { };

    public:
        void claim(const Tuple& tuple);
        void add_handler(const Handler& handler);
        void tick();


    };

} // foxtalk

#endif //REACTOR_REACTOR_H
