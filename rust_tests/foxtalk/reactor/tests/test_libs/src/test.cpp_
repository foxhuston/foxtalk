
// extern "C" pub fn wish(tuple: Tuple) {

#include <vector>
#include "c/reactor.hpp"

static char* lexi = "lexi";
static char* highlighted = "is highlighted";
static char* blue = "blue";


///// A HANDLER FILE ///////////////////////////////////////////////////////////

#include <string>
#include <iostream>

struct Dog {
    std::string name;
};

Tuple GetQuery() {
    return Tuple {
        nullptr, "Hi!", nullptr
    };
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from C++ WhenHandler" << std::endl;

    auto subj = new TupleNoun {
        TupleNoun::Tag::Str,
        { .str = lexi }
    };

    auto obj = new TupleNoun {
        TupleNoun::Tag::Ptr,
        { .ptr = new Dog { "lexi" } }
    };

    Tuple outTuple { subj, "is a", obj };

    auto out = new std::vector<Tuple> {
        outTuple
    };
    return out;
}

extern "C" void free_tuple_obj(TupleNoun* n) {
//    std::cout << "Cleanin' up some dogs." << std::endl;
//    delete static_cast<Dog *>(n->dat.ptr);
//    delete n;
}

extern "C" void free_tuple_subj(TupleNoun* subj) {
//    delete subj;
}

