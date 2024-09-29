#ifndef __REACTOR_TYPES__
#define __REACTOR_TYPES__

#include <cstdint>

extern "C" {
    struct TupleNoun  {
        enum Tag {
            Ptr,
            Str,
            U64,
            I64
        };

        Tag tag;
        union {
            void* ptr;
            char* str;
            uint64_t u64;
            int64_t i64;
        } dat;
    };

    struct Tuple {
        TupleNoun *subject;
        char *predicate;
        TupleNoun *object;
    };
}

#endif // __REACTOR_TYPES__
