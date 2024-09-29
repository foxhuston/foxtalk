#ifndef __REACTOR__
#define __REACTOR__

#include <vector>
#include "reactor_types.hpp"

std::vector<Tuple>* WhenHandler(Tuple* result);
Tuple GetQuery();

extern "C" {
    void get_query(Tuple* t) {
        auto q = GetQuery();
        t->subject = q.subject;
        t->predicate = q.predicate;
        t->object = q.object;
    }

    Tuple* when_handler(Tuple* result, size_t *outLen) {
        auto res = WhenHandler(result);
        *outLen = res->size();
        return res->data();
    }
}

#endif // __REACTOR__
