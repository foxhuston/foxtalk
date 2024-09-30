#include <cstring>
#include <iostream>

typedef void* TupleNoun;
typedef void* Tuple;

extern "C" {
    TupleNoun mk_tuple_noun_query();
    TupleNoun mk_tuple_noun_symbol(const char *s);
    TupleNoun mk_tuple_noun_ptr();
    Tuple mk_tuple(TupleNoun subject, TupleNoun predicate, TupleNoun object);
    TupleNoun get_tuple_subject(Tuple t);
    const char *get_tuple_noun_as_string(TupleNoun tn);

}  // extern "C"

extern "C" Tuple GetQuery()
{
    char description[32] {};
    strcpy(description, "description");

    return mk_tuple(
        mk_tuple_noun_symbol(description),
        mk_tuple_noun_query(),
        mk_tuple_noun_query()
    );
}