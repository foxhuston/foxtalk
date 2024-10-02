//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_HANDLER_H
#define REACTOR_HANDLER_H

#include "Tuple.h"

namespace foxtalk {
    class Handler {
    public:
        virtual Tuple* get_query() const = 0;
        virtual void handle_results(TupleVec tv) const = 0;
    };
}

#endif //REACTOR_HANDLER_H
