#include <iostream>
#include "foxtalk.h"

static char* camera = "camera";

Tuple GetQuery() {
    return Tuple {
        nullptr
        , "is a"
        , static_cast<void*>(camera)
    };
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from CameraHandler" << std::endl;
    return {};
}

extern "C" void free_tuple_obj(void* obj) {}

extern "C" void free_tuple_subj(void* subj) {}

