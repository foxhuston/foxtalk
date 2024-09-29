#ifndef __REACTOR_TYPES__
#define __REACTOR_TYPES__

#include <cstdint>
#include <iostream>

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
    };

    struct Tuple {
        TupleNoun subject;
        char *predicate;
        TupleNoun object;
    };

//class tn {
//private:
//    const TupleNoun& t;
//
//public:
//    tn(const TupleNoun& t): t { t } {}
//
//    friend std::ostream& operator<< (std::ostream &os, const tn& t) {
//        switch(t.t.tag) {
//            case TupleNoun::Ptr:
//                os << "Ptr(" << t.t.dat.ptr << ")";
//                break;
//            case TupleNoun::Str:
//                os << "Str(" << t.t.dat.str << ")";
//                break;
//            case TupleNoun::U64:
//                os << "U64(" << t.t.dat.u64 << ")";
//                break;
//            case TupleNoun::I64:
//                os << "I64(" << t.t.dat.i64 << ")";
//                break;
//        }
//        return os;
//    }
//};

#endif // __REACTOR_TYPES__
