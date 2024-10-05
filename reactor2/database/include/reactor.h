#ifndef __FOXTALK_REACTOR_H__
#define __FOXTALK_REACTOR_H__

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif


    typedef struct Reactor_t {

    } Reactor;

    Reactor* mkReactor();

    Tuple *reactor_addTuple(Reactor *, TupleNoun subject, TupleNoun predicate, TupleNoun object);
    void reactor_removeTuple(Reactor *, Tuple *);


#ifdef __cplusplus
}
#endif

#endif // __FOXTALK_REACTOR_H__
