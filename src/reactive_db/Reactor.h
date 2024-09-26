//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_REACTOR_H
#define FOXTALK_REACTOR_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <functional>
#include "Tuple.h"

typedef void (*wish)(Tuple fact);
//typedef void (*handler)(wish wish, size_t nBindings, Tuple* bindings);
typedef std::function<void(wish, size_t, Tuple*)> handler;

class Reactor {
    typedef std::map<Symbol*, std::set<Tuple*>> SymbolMap;
    typedef std::map<void*, std::set<Tuple*>> VoidMap;

    Symbol symbols;

    VoidMap _facts_by_subject;
    SymbolMap _facts_by_predicate;
    VoidMap _facts_by_object;

    std::set<Tuple *> query(Tuple q);

public:
    void claim(Tuple *tuple);
    void removeClaim(Tuple *tuple);

    // TODO: Make the query much more flexible
    void when(Tuple query, handler handler);
    void removeWhen(handler handler);

    void tick();

    Symbol* symbol(std::string s) {
        return symbols.intern(s);
    }
};


#endif //FOXTALK_REACTOR_H
