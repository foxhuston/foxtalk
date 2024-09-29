#include <vector>
#include <string>
#include <iostream>

#include "reactor.hpp"

static char *isA = "is a";
static char* camera = "camera";

Tuple GetQuery() {
    return Tuple {
        { TupleNoun::Tag::Query },
        isA,
        { TupleNoun::Tag::Str, { .str = camera }}
    };
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from CameraHandler" << std::endl;
    std::cout << "    Subject is " << result->subject << std::endl;
    return new std::vector<Tuple>();
}

extern "C" void free_tuple_obj(void* obj) {}

extern "C" void free_tuple_subj(void* subj) {}

