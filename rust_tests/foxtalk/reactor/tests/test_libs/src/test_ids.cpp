
// extern "C" pub fn wish(tuple: Tuple) {

#include <vector>
#include <string>
#include <iostream>

#include "c/reactor.hpp"

static char *husky = "husky";

struct Dog
{
    std::string name;
};

Tuple GetQuery()
{
    auto obj = new TupleNoun {
        TupleNoun::Tag::Str,
        { .str = husky }
    };

    return Tuple { nullptr, "is a", obj };
}

// When (you) is a "husky"
std::vector<Tuple> *WhenHandler(Tuple *result)
{
    std::cout << "Hello from C++ WhenHandler" << std::endl;

    auto obj = new TupleNoun {
        TupleNoun::Tag::Ptr,
        { .ptr = new Dog { static_cast<char *>(result->subject->dat.str) } }
    };

    Tuple outTuple {
        result->subject, "is a", obj
    };

    auto out = new std::vector<Tuple>{outTuple};

    return out;
}

extern "C" void free_tuple_obj(void *obj)
{
    std::cout << "Cleanin' up some dogs." << std::endl;
    delete static_cast<Dog *>(obj);
}

extern "C" void free_tuple_subj(void *subj) {}
