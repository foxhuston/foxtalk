//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_REACTIVEDB_H
#define FOXTALK_REACTIVEDB_H

/*
 * Facts (changeable)
 * When clauses (changeable)
 *   -> Can update Facts
 *
 * tick-based
 * tuples have provenance
 *
 *
 *
 */

#include <vector>
#include <optional>
#include <string>

#include "Symbol.h"

using namespace std;

class ReactiveDb {
private:
    // Do it dumb the first time
    vector<tuple<void*, string, void*>> _triples;
    vector<void*> _whens;

    Symbol _symbols;

public:
    ReactiveDb() : _triples {}, _whens {}, _symbols {}
    {}

    void claim(string subj, string pred, void* obj) {
        claim(_symbols.intern(subj), pred, obj);
    }

    void claim(void* subj, string pred, string obj) {
        claim(subj, pred, _symbols.intern(obj));
    }

    void claim(string subj, string pred, string obj) {
        claim(_symbols.intern(subj), pred, _symbols.intern(obj));
    }

    void claim(void* subj, string pred, void* obj) {
        _triples.push_back(tuple(subj, pred, obj));
    }

    vector<tuple<void*, string, void*>> query(optional<void*> subj, optional<string> pred, optional<void*> obj);

    // Really not the right way to do this part...
    vector<tuple<void*, string, void*>> query(string subj, optional<string> pred, optional<void*> obj) {
        return query(_symbols.intern(subj), pred, obj);
    }

    vector<tuple<void*, string, void*>> query(optional<void*> subj, optional<string> pred, string obj) {
        return query(subj, pred, _symbols.intern(obj));
    }

    vector<tuple<void*, string, void*>> query(string subj, optional<string> pred, string obj) {
        return query(_symbols.intern(subj), pred, _symbols.intern(obj));
    }
};


#endif //FOXTALK_REACTIVEDB_H
