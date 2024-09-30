#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

struct TupleNoun;

struct PtrTuple {
  TupleNoun *subject;
  TupleNoun *predicate;
  TupleNoun *object;
};

struct Tuple {
  TupleNoun subject;
  TupleNoun predicate;
  TupleNoun object;
};

extern "C" {

TupleNoun *mk_tuple_noun_symbol(const char *s);

TupleNoun *mk_tuple_noun_ptr(void *data, void (*free_fn)(void*));

TupleNoun *mk_tuple_noun_query();

PtrTuple *mk_tuple(TupleNoun *subject, TupleNoun *predicate, TupleNoun *object);

TupleNoun get_tuple_subject(Tuple t);

const char *get_tuple_noun_as_string(TupleNoun tn);

}  // extern "C"
