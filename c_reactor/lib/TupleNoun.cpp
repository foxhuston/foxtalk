//
// Created by fox on 10/3/24.
//

#include <iostream>
#include <boost/functional/hash.hpp>

#include "gc.h"
#include "TupleNoun.h"

namespace foxtalk {

    TupleNoun::~TupleNoun() {
        std::cout << "DEBUG: Freeing Tuple Noun..." << std::endl;
        if (type == CPtr) {
            data.cptr.free_fn(data.cptr.data);
        }
    }

    bool TupleNoun::operator==(const TupleNoun &rhs) const {
        if (type == rhs.type) {
            switch (type) {
                case Type::Query:
                    return true;
                case Type::Pair:
                    return data.pair.fst == rhs.data.pair.fst
                           && data.pair.snd == rhs.data.pair.snd;
                case Type::Symbol:
                    return strcmp(data.symbol, rhs.data.symbol) == 0;
                case Type::CPtr:
                    return data.cptr.data == rhs.data.cptr.data
                           && data.cptr.free_fn == rhs.data.cptr.free_fn;
                case Type::U64:
                    return data.u64 == rhs.data.u64;
                case Type::I64:
                    return data.i64 == rhs.data.i64;
            }
        }

        return false;
    }

    const size_t TupleNoun::hash() const {
        size_t seed = 0;
        boost::hash_combine(seed, type);

        switch (type) {
            case Type::Query:
                break;
            case Type::Pair:
                boost::hash_combine(seed, data.pair.fst);
                boost::hash_combine(seed, data.pair.snd);
                break;
            case Type::Symbol:
                boost::hash_combine(seed, std::string(data.symbol));
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

    std::ostream &operator<<(std::ostream &os, const TupleNoun &noun) {
        switch (noun.type) {
            case TupleNoun::Type::Query:
                os << "Query";
                break;
            case TupleNoun::Type::Pair:
                os << "<" << noun.data.pair.fst << ", " << noun.data.pair.snd << ">";
                break;
            case TupleNoun::Type::Symbol:
                os << noun.data.symbol;
                break;
            case TupleNoun::Type::CPtr:
                os << "{CPtr " << noun.data.cptr.data << "}";
                break;
            case TupleNoun::Type::U64:
                os << noun.data.u64;
                break;
            case TupleNoun::Type::I64:
                os << noun.data.i64;
                break;
        }

        return os;
    }

} //foxtalk

static void FinalizeCptrTupleNoun(GC_PTR void_obj, GC_PTR void_env) {
    auto tn = (foxtalk::TupleNoun *) void_obj;
    if (tn->is_cptr()) {
        tn->data.cptr.free_fn(tn->data.cptr.data);
    } else {
        std::cout << "WARNING! GC Trying to finalize non-cptr tuple noun!" << std::endl;
    }
}

extern "C" {

foxtalk::TupleNoun *mkQuery() {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc_atomic(sizeof(foxtalk::TupleNoun), "QueryNoun", 0);
    tn->type = foxtalk::TupleNoun::Type::Query;
    tn->data.u64 = 0;
    return tn;
}

foxtalk::TupleNoun *mkPair(foxtalk::TupleNoun *a, foxtalk::TupleNoun *b) {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc(sizeof(foxtalk::TupleNoun), "PairNoun", 0);
    tn->type = foxtalk::TupleNoun::Type::Pair;
    tn->data.pair.fst = a;
    tn->data.pair.snd = b;
    return tn;
}

foxtalk::TupleNoun *mkSymbol(const char *s) {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc(sizeof(foxtalk::TupleNoun), "SymbolNoun", 0);
    tn->type = foxtalk::TupleNoun::Type::Symbol;
    tn->data.symbol = GC_strdup(s);
    return tn;
}

foxtalk::TupleNoun *mkPtr(void *dat, foxtalk::TupleNoun::Free_Fn free_fn) {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc(sizeof(foxtalk::TupleNoun), "CPtrNoun", 0);
    tn->type = foxtalk::TupleNoun::Type::CPtr;
    tn->data.cptr.data = dat;
    tn->data.cptr.free_fn = free_fn;

    GC_finalization_proc old_proc;
    size_t my_env;
    void *old_env;
    // TODO: Hmm...
    GC_register_finalizer(tn, FinalizeCptrTupleNoun, &my_env, &old_proc, &old_env);

    return tn;
}

foxtalk::TupleNoun *mkU64(uint64_t n) {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc_atomic(sizeof(foxtalk::TupleNoun), "U64Noun", 0);
    tn->type = foxtalk::TupleNoun::Type::U64;
    tn->data.u64 = n;

    return tn;
}

foxtalk::TupleNoun *mkI64(int64_t n) {
    auto tn = (foxtalk::TupleNoun *) GC_debug_malloc_atomic(sizeof(foxtalk::TupleNoun), "I64Noun", 0);
    tn->type = foxtalk::TupleNoun::Type::I64;
    tn->data.i64 = n;

    return tn;
}
}
