//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_TUPLE_H
#define REACTOR_TUPLE_H

#include <iostream>
#include <format>

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

  Tuple(const TupleNoun *subject, const TupleNoun *predicate,
        const TupleNoun *object)
      : subject{subject}, predicate{predicate}, object{object} {}

  static Tuple *mk(const TupleNoun *subject, const TupleNoun *predicate,
                   const TupleNoun *object);

  ~Tuple();

  bool operator==(const Tuple &other) const {
//    std::cout << "Tuple Compare: "
//              << "    s: " << subject << " == " << other.subject << std::endl
//              << "    p: " << predicate << " == " << other.predicate << std::endl
//              << "    o: " << object << " == " << other.object << std::endl;

    return *subject == *other.subject && *predicate == *other.predicate &&
           *object == *other.object;
  };

  [[nodiscard]] const TupleNoun *getSubject() const { return subject; }

  [[nodiscard]] const TupleNoun *getPredicate() const { return predicate; }

  [[nodiscard]] const TupleNoun *getObject() const { return object; }

  friend std::ostream &operator<<(std::ostream &os, const Tuple &tuple) {
    os << "<" << *tuple.subject << ", " << *tuple.predicate << ", "
       << *tuple.object << ">";
    return os;
  }
};

std::size_t hash_value(const TupleNoun *t);
std::size_t hash_value(const Tuple *t);

} // namespace foxtalk

template<>
struct std::formatter<foxtalk::Tuple> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  auto format(const foxtalk::Tuple& obj, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "<{}, {}, {}>",
                          *obj.getSubject(),
                          *obj.getPredicate(),
                          *obj.getObject());
  }
};

#endif // REACTOR_TUPLE_H
