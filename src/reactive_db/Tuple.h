//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_TUPLE_H
#define FOXTALK_TUPLE_H

#include "Symbol.h"

struct Tuple {
    void* subject;
    Symbol* predicate;
    void* object;
    Tuple* provenance;

    bool operator==(const Tuple& rhs) const {
        return subject == rhs.subject
            && predicate == rhs.predicate
            && object == rhs.object
            && *provenance == *rhs.provenance;
    }

    friend std::ostream& operator<<(std::ostream& os, const Tuple& sym) {
        os << "<";
        if(sym.subject == nullptr) { os << "_, "; } else { os << sym.subject << ", "; }
        if(sym.predicate == nullptr) { os << "_, "; } else { os << *sym.predicate << ", "; }
        if(sym.object == nullptr) { os << "_>"; } else { os << sym.object << ">"; }
        return os;
    }
};

#endif //FOXTALK_TUPLE_H
