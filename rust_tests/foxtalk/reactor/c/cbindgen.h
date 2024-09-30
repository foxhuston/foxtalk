#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

struct String;

struct TupleNoun {
  enum class Tag {
    CPtrHeap,
    CPtr,
    Str,
    U64,
    I64,
  };

  struct CPtrHeap_Body {
    void *data;
    void *destructor;
  };

  struct CPtr_Body {
    void *_0;
  };

  struct Str_Body {
    String _0;
  };

  struct U64_Body {
    uint64_t _0;
  };

  struct I64_Body {
    int64_t _0;
  };

  Tag tag;
  union {
    CPtrHeap_Body c_ptr_heap;
    CPtr_Body c_ptr;
    Str_Body str;
    U64_Body u64;
    I64_Body i64;
  };
};

struct PtrTuple {
  TupleNoun *subject;
  TupleNoun *predicate;
  TupleNoun *object;
};

extern "C" {

TupleNoun *mk_tuple_noun_symbol(const char *s);

TupleNoun *mk_tuple_noun_query();

PtrTuple *mk_tuple(TupleNoun *subject, TupleNoun *predicate, TupleNoun *object);

}  // extern "C"
