
// extern "C" pub fn wish(tuple: Tuple) {

#include <vector>

static char* lexi = "lexi";
static char* highlighted = "is highlighted";
static char* blue = "blue";

///// Header File //////////////////////////////////////////////////////////////

extern "C" {
    struct Tuple {
        void* subject;
        const char *predicate;
        void* object;
    };
}

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

    Tuple outTuple {
        static_cast<void *>(lexi)
        , "is a"
        , new Dog { "lexi" }
    };
    auto out = new std::vector<Tuple> {
        outTuple
    };
    return out;
}

extern "C" void free_tuple_obj(void* obj) {
    std::cout << "Cleanin' up some dogs." << std::endl;
    delete static_cast<Dog *>(obj);
}

extern "C" void free_tuple_subj(void* subj) {}

