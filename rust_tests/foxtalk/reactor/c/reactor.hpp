#ifndef __REACTOR__
#define __REACTOR__

#include <vector>
#include "reactor_types.hpp"

///// REQUIRED IMPLEMENTATIONS /////////////////////////////////////////////////
std::vector<Tuple>* WhenHandler(Tuple* result);
Tuple GetQuery();

extern "C" {
    ///// HANDLER API //////////////////////////////////////////////////////////
    void get_query(Tuple* t) {
        auto q = GetQuery();
        t->subject = q.subject;
        t->predicate = q.predicate;
        t->object = q.object;
    }

    Tuple* when_handler(Tuple* result, size_t *outLen) {
        try {
            auto res = WhenHandler(result);
            *outLen = res->size();
            return res->data();
        } catch(std::runtime_error &e) {
            std::cerr << "Critical Exception: " << e.what() << std::endl;
            return nullptr;
        }
    }

    ///// INTERNAL RUST<--> C NONSENSE /////////////////////////////////////////
}

#endif // __REACTOR__
