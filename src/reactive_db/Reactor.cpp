//
// Created by fox on 9/26/24.
//

#include <algorithm>
#include <iostream>

#include "Reactor.h"

template<typename K, typename V>
inline void mm_insert(std::map<K, std::set<V>>& m, K k, V v) {
    if(!m.contains(k)) {
        m.insert({ k, { v } });
    } else {
        m.find(k)->second.insert(v);
    }
}

template<typename K, typename V>
inline std::set<V> mm_find(std::map<K, std::set<V>>& m, K k) {
    if(!m.contains(k)) {
        m.insert({ k, {} });
    }

    return m.find(k)->second;
}

template<typename K, typename V>
inline void mm_remove(std::map<K, std::set<V>>& m, K k, V v) {
    if(!m.contains(k)) {
        return;
    }

    m.find(k)->second.erase(v);
}

void Reactor::claim(Tuple *tuple) {
    mm_insert(_facts_by_subject, tuple->subject, tuple);
    mm_insert(_facts_by_predicate, tuple->predicate, tuple);
    mm_insert(_facts_by_object, tuple->object, tuple);
    if(tuple->provenance != nullptr) {
        std::cout << "Inserting tuple with provenance!" << std::endl;
        mm_insert(_facts_by_provenance, tuple->provenance, tuple);
    }
}

void Reactor::removeClaim(Tuple *tuple) {
    mm_remove(_facts_by_subject, tuple->subject, tuple);
    mm_remove(_facts_by_predicate, tuple->predicate, tuple);
    mm_remove(_facts_by_object, tuple->object, tuple);
    for(auto fact : mm_find(_facts_by_provenance, tuple)) {
        removeClaim(fact);
    }
}

void Reactor::when(Tuple query, handler handler) {
    handlers.push_back({ query, handler });
}

// TODO: This is ridiculous :/
//       Why doesn't the set object have quick intersection??
std::set<Tuple *> Reactor::query(Tuple q) {
    std::optional<std::set<Tuple *>> outputSet;

    if(q.subject != nullptr) {
        outputSet = mm_find(_facts_by_subject, q.subject);
    }

    if(q.predicate != nullptr) {
        auto res = mm_find(_facts_by_predicate, q.predicate);
        if(outputSet.has_value()) {
            std::vector<Tuple *> newOut {};
            std::set_intersection(
                    outputSet.value().begin(), outputSet.value().end()
                    , res.begin(), res.end()
                    , std::back_inserter(newOut));

            outputSet = std::set<Tuple *> { newOut.begin(), newOut.end() };
        } else {
            outputSet = res;
        }
    }

    if(q.object != nullptr) {
        auto res = mm_find(_facts_by_object, q.object);
        if(outputSet.has_value()) {
            std::vector<Tuple *> newOut {};
            std::set_intersection(
                    outputSet.value().begin(), outputSet.value().end()
                    , res.begin(), res.end()
                    , std::back_inserter(newOut));

            // TODO: This is ridiculous :/
            outputSet = std::set<Tuple *> { newOut.begin(), newOut.end() };
        } else {
            outputSet = res;
        }
    }

    return outputSet.has_value() ? outputSet.value() : std::set<Tuple*> {};
}

void Reactor::tick() {
    for(auto [q, h] : handlers) {
        std::cout << "checking handler for tuple" << q << std::endl;
        auto results = query(q);
        std::cout << "  got " << results.size() << " result(s)" << std::endl;

        for(auto result : results) {
            h([this, result](Tuple *fact) {
                std::cout << "Wishing " << *fact << " with provenance " << *result << std::endl;
                fact->provenance = result;
                this->claim(fact);
            }, 1, result);
        }
    }
}
