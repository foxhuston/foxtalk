#include <cstring>
#include <iostream>

#include "reactor.h"

Tuple GetQuery()
{
    char description[32] {};
    strcpy(description, "description");

    return mk_tuple(
        mk_tuple_noun_symbol(description),
        mk_tuple_noun_query(),
        mk_tuple_noun_query()
    );
}