//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_TUPLE_H
#define FOXTALK_TUPLE_H

#import "Symbol.h"

struct Tuple {
    void* subject;
    Symbol* predicate;
    void* object;

    Tuple* provenance;
};

#endif //FOXTALK_TUPLE_H
