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
    ) {
        vector<tuple<void*, string, void*>> out {};

        for(const auto& tuple : _triples) {
            auto& [tsubj, tpred, tobj] = tuple;
            auto isMatch =
                    (!subj.has_value() || (subj.has_value() && subj.value() == tsubj))
                    && (!pred.has_value() || (pred.has_value() && pred.value() == tpred))
                    && (!obj.has_value() || (obj.has_value() && obj.value() == tobj));

            if(isMatch) {
                out.push_back(tuple);
            }
        }

        return out;
    }

};


#endif //FOXTALK_REACTIVEDB_H
