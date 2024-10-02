#include "reactor.h"

Tuple GetQuery()
{
    return mk_tuple(
        mk_tuple_noun_query(),
        mk_tuple_noun_symbol("is a"),
        mk_tuple_noun_symbol("husky")
    );
}

std::vector<Tuple> *WhenHandler(Tuple result)
{
    auto subj = get_tuple_subject(result);

    auto out = new std::vector<Tuple>{};

    out->push_back(mk_tuple(
        subj,
        mk_tuple_noun_symbol("is"),
        mk_tuple_noun_symbol("cool")
    ));

    std::cout << "a " << subj << std::endl;

    std::cout << "b " << subj << std::endl;

    out->push_back(mk_tuple(
        subj,
        mk_tuple_noun_symbol("is a"),
        mk_tuple_noun_symbol("pup")
    ));

    std::cout << "c " << subj << std::endl;

    return out;
}
