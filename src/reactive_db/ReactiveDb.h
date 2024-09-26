//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_REACTIVEDB_H
#define FOXTALK_REACTIVEDB_H

#include <vector>
#include <optional>
#include <string>

using namespace std;

class ReactiveDb {
private:
    // Do it dumb the first time
    vector<tuple<void*, string, void*>> _triples;

public:
    ReactiveDb() : _triples {}
    {}

    void claim(void* subj, string pred, void* obj) {
        _triples.push_back(tuple(subj, pred, obj));
    }

    vector<tuple<void*, string, void*>> query(
        optional<void*> subj,
        optional<string> pred,
        optional<void*> obj
    );

};


#endif //FOXTALK_REACTIVEDB_H
