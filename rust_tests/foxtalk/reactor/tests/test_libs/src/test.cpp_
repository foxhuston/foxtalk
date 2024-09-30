
// extern "C" pub fn wish(tuple: Tuple) {

#include <vector>
#include <string>
#include <iostream>

#include "c/reactor.hpp"

static char *hi = "Hi!";
static char *husky = "husky";

struct Dog
{
    std::string name;
};

Tuple GetQuery()
{
    return Tuple {
        { TupleNoun::Tag::Query },
        hi,
        { TupleNoun::Tag::Str, { .str = husky }}
    };
}

// When (you) is a "husky"
std::vector<Tuple> *WhenHandler(Tuple *result)
{
    std::cout << "Hello from C++ WhenHandler" << std::endl;
    std::cout << "Subject is " << result->subject << std::endl;

    auto obj = new TupleNoun {
        TupleNoun::Tag::Ptr,
        // Needs copy??
        { .ptr = new Dog { result->subject.dat.str } }
    };

    Tuple outTuple {
        result->subject,
        hi,
        // Needs string copy??
        { TupleNoun::Tag::Ptr, { .ptr = new Dog { result->subject.dat.str } } }
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
