//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "gc_cpp.h"

#include "Db.h"
#include "Tuple.h"
#include "Handler.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor : gc {
    private:
        Db db {};
        std::vector<const Handler *> handlers { };

    public:
        void claim(const Tuple& tuple);
        void add_handler(const Handler* handler);
        void tick();


    };

} // foxtalk

#endif //REACTOR_REACTOR_H
