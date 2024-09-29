#ifndef __REACTOR_TYPES__
#define __REACTOR_TYPES__

#include <cstdint>
#include <iostream>
#include <cstring>

struct TupleNoun  {
    enum Tag {
        Query,
        Ptr,
        Str,
        U64,
        I64
    };

    Tag tag;
    union Dat {
        void* ptr;
        char* str;
        uint64_t u64;
        int64_t i64;
    } dat;

    friend std::ostream & operator << (std::ostream &os, const TupleNoun& t) {
        switch(t.tag) {
            case Query:
                os << "Query";
                break;
            case Ptr:
                os << "Ptr(" << t.dat.ptr << ")";
                break;
            case Str:
                os << "Str(" << t.dat.str << ")";
                break;
            case U64:
                os << "U64(" << t.dat.u64 << ")";
                break;
            case I64:
                os << "I64(" << t.dat.i64 << ")";
                break;
        }
        return os;
    }

    static TupleNoun fromUint(uint64_t n) {
        return TupleNoun {
            Tag::U64,
            { .u64 = n }
        };
    }

    static TupleNoun fromString(std::string string) {
        // TODO: SO DAMN MEMORY-LEAKY :<
        char *str = (char *)malloc(sizeof(char) * string.length());
        strncpy(str, string.c_str(), string.length());

        return TupleNoun {
            Tag::Str,
            { .str = str }
        };
    }
};

struct Tuple {
    TupleNoun subject;
    char *predicate;
    TupleNoun object;
};

#endif // __REACTOR_TYPES__
