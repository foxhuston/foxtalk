//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_TUPLE_H
#define REACTOR_TUPLE_H

#include "TupleNoun.h"

namespace foxtalk {
    struct Tuple {
    private:
        const TupleNoun *subject;
        const TupleNoun *predicate;
        const TupleNoun *object;

    public:
//        Tuple(Tuple &) = delete;
//        Tuple(Tuple &&) = delete;
        Tuple(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object) = delete;

        static Tuple* mk(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object);

        bool operator==(const Tuple& other) const {
            return *subject == *other.subject
                && *predicate == *other.predicate
                && *object == *other.object;
        };

        [[nodiscard]] const TupleNoun* getSubject() const { return subject; }

        [[nodiscard]] const TupleNoun* getPredicate() const { return predicate; }

        [[nodiscard]] const TupleNoun* getObject() const { return object; }

        friend std::ostream &operator<<(std::ostream &os, const Tuple &tuple) {
            os <<  "<" << *tuple.subject << ", " << *tuple.predicate
               << ", " << *tuple.object << ">";
            return os;
        }
    };

    std::size_t hash_value(const TupleNoun* t);
    std::size_t hash_value(const Tuple* t);
}

#endif //REACTOR_TUPLE_H
