//
// Created by fox on 10/3/24.
//

#ifndef REACTOR_TUPLENOUN_H
#define REACTOR_TUPLENOUN_H

#include <cstdint>
#include <ostream>

namespace foxtalk {
    struct TupleNoun {
        typedef void (*Free_Fn)(void *);

    private:
        enum Type {
            Query,
            Pair,
            Symbol,
            CPtr,
            U64,
            I64
        } type;

    public:
        struct CPtrWithFree {
            void *data;
            Free_Fn free_fn;
        };

        struct TuplePair {
            TupleNoun *fst;
            TupleNoun *snd;
        };

        union Data {
            TuplePair pair;
            char *symbol;
            CPtrWithFree cptr;
            uint64_t u64;
            int64_t i64;
        } data;

//        TupleNoun(Type type, Data data) : type{type}, data{data} {}
        TupleNoun() = delete;
        TupleNoun(Type type, Data data) = delete;

        static TupleNoun *mkQuery();

        static TupleNoun *mkPair(TupleNoun *a, TupleNoun *b);

        static TupleNoun *mkSymbol(const char *s);

        static TupleNoun *mkSymbol(char *s);

        static void FinalizeCptrTupleNoun(void* void_obj, void* void_env);

        static TupleNoun *mkPtr(void *dat, Free_Fn free_fn);

        static TupleNoun *mkU64(uint64_t n);

        static TupleNoun *mkI64(int64_t n);

        ~TupleNoun();

        bool operator==(const TupleNoun &rhs) const;

        [[nodiscard]] const size_t hash() const;

        [[nodiscard]] bool is_query() const { return type == Type::Query; }

        [[nodiscard]] bool is_pair() const { return type == Type::Pair; }

        [[nodiscard]] bool is_cptr() const { return type == Type::CPtr; }

        [[nodiscard]] bool is_symbol() const { return type == Type::Symbol; }

        [[nodiscard]] bool is_u64() const { return type == Type::U64; }

        [[nodiscard]] bool is_i64() const { return type == Type::I64; }

    public:
        friend std::ostream &operator<<(std::ostream &os, const TupleNoun &noun);
    };

} // foxtalk

#endif //REACTOR_TUPLENOUN_H
