//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "gc.h"
#include "Tuple.h"

// GC_malloc & GC_register_finalizer will be the tricks, here.

namespace foxtalk {

    class Reactor : gc {

    };

} // foxtalk

#endif //REACTOR_REACTOR_H
