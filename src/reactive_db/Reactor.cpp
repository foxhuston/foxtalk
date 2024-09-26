//
// Created by fox on 9/26/24.
//

#include <algorithm>

#include "Reactor.h"

template<typename K, typename V>
void mm_insert(std::map<K, std::set<V>> m, K k, V v) {
    if(!m.contains(k)) {
        m.insert({ k, {} });
    }

    m.find(k)->second.insert(v);
}

template<typename K, typename V>
std::set<V> mm_find(std::map<K, std::set<V>> m, K k) {
    if(!m.contains(k)) {
        m.insert({ k, {} });
    }

    return m.find(k)->second;
}

void Reactor::claim(Tuple *tuple) {
    mm_insert(_facts_by_subject, tuple->subject, tuple);
    mm_insert(_facts_by_predicate, tuple->predicate, tuple);
    mm_insert(_facts_by_object, tuple->object, tuple);
}

void Reactor::removeClaim(Tuple *tuple) {
    // should this `delete` the tuple?
    throw std::runtime_error("unimplemented.");
}

void Reactor::when(Tuple query, handler handler) {

}

std::set<Tuple *> Reactor::query(Tuple q) {
    std::optional<std::set<Tuple *>> outputSet;

    if(q.subject != nullptr) {
        outputSet = mm_find(_facts_by_subject, q.subject);
    }

    if(q.predicate != nullptr) {
        auto res = mm_find(_facts_by_predicate, q.predicate);
        if(outputSet.has_value()) {
            std::set<Tuple *> newOut {};
            std::set_intersection(
                    outputSet.value().begin(), outputSet.value().end()
                    , res.begin(), res.end()
                    , std::back_inserter(newOut));
        }
    }

    if(q.object != nullptr) {
        intersections.push_back(mm_find(_facts_by_object, q.object));
    }
}
