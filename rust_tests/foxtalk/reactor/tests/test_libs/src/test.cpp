
// extern "C" pub fn wish(tuple: Tuple) {

#include <vector>
#include <string>
#include <iostream>

#include "reactor.h"

void free_dog(void *obj);

struct Dog
{
    std::string name;
};

Tuple GetQuery()
{
    return mk_tuple(
        mk_tuple_noun_query(),
        mk_tuple_noun_symbol("is a"),
        mk_tuple_noun_symbol("husky")
    );
}

// When (you) is a "husky"
std::vector<Tuple> *WhenHandler(Tuple result)
{
    std::cout << "Hello from C++ WhenHandler" << std::endl;

    auto subj = get_tuple_subject(result);
    auto outTuple = mk_tuple(
        subj,
        mk_tuple_noun_symbol("is a"),
        mk_tuple_noun_cptr_with_free(
            new Dog { get_tuple_noun_as_symbol(subj) },
            free_dog
        )
    );

    auto out = new std::vector<Tuple>{outTuple};
    return out;
}

void free_dog(void *obj)
{
    std::cout << "Cleanin' up some dogs." << std::endl;
    delete static_cast<Dog *>(obj);
}

extern "C" void free_tuple_subj(void *subj) {}
