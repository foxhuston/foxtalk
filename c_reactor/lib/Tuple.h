//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_TUPLE_H
#define REACTOR_TUPLE_H

#include "gc.h"
#include "gc_cpp.h"
#include <cstdint>
#include "boost/container_hash/hash.hpp"

namespace foxtalk {
    struct TupleNoun : gc {
        typedef void (*Free_Fn)(void *);

        static TupleNoun* mkQuery() {
            return {};
        }

        static TupleNoun* mkSymbol(const char *s) {
            return new TupleNoun {Symbol, {.symbol = GC_strdup(s) }};
        }

        static TupleNoun* mkSymbol(char *s) {
            return new TupleNoun {Symbol, {.symbol = GC_strdup(s) }};
        }

        static TupleNoun* mkPtr(void *dat, Free_Fn free_fn) {
            return new TupleNoun {CPtr, {.cptr = {dat, free_fn}}};
        }

        static TupleNoun* mkU64(uint64_t n) {
            return new TupleNoun {U64, {.u64 = n}};
        }

        static TupleNoun* mkI64(int64_t n) {
            return new TupleNoun {I64, {.i64 = n}};
        }

        ~TupleNoun() {
            if (type == CPtr) {
                data.cptr.free_fn(data.cptr.data);
            }
        }

        const size_t hash() const {
            size_t seed = 0;
            switch (type) {
                case Type::Query:
                    break;
                case Type::Symbol:
                    boost::hash_combine(seed, data.symbol);
                    break;
                case Type::CPtr:
                    boost::hash_combine(seed, data.cptr.data);
                    boost::hash_combine(seed, data.cptr.free_fn);
                    break;
                case Type::U64:
                    boost::hash_combine(seed, data.u64);
                    break;
                case Type::I64:
                    boost::hash_combine(seed, data.i64);
                    break;
            }

            return seed;
        }

    private:
        enum Type {
            Query,
            Symbol,
            CPtr,
            U64,
            I64
        } type;

        struct CPtrWithFree {
            void *data;
            Free_Fn free_fn;
        };

        union Data {
            char *symbol;
            CPtrWithFree cptr;
            uint64_t u64;
            int64_t i64;
        } data;

        TupleNoun() : type{Query}, data{} {}

        TupleNoun(Type type, Data data) : type{type}, data{data} {}

    };

    struct Tuple : public gc {
    private:
        const TupleNoun *subject;
        const TupleNoun *predicate;
        const TupleNoun *object;

    public:
//        Tuple(Tuple &) = delete;
//        Tuple(Tuple &&) = delete;

        bool operator==(const Tuple& other) const {
            return subject == other.subject
                && predicate == other.predicate
                && object == other.object;
        };

        Tuple(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object)
                : subject(subject),
                  predicate(predicate),
                  object(object) {}

        const TupleNoun* getSubject() const { return subject; }

        const TupleNoun* getPredicate() const { return predicate; }

        const TupleNoun* getObject() const { return object; }
    };
}

template<>
struct std::hash<foxtalk::TupleNoun> {
    size_t operator()(const foxtalk::TupleNoun& t) const noexcept {
        return t.hash();
    }
};

template<>
struct std::hash<foxtalk::Tuple> {
    size_t operator()(const foxtalk::Tuple& t) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, t.getSubject());
        boost::hash_combine(seed, t.getPredicate());
        boost::hash_combine(seed, t.getObject());
        return seed;
    }
};

#endif //REACTOR_TUPLE_H
