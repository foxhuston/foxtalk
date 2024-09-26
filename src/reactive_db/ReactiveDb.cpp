//
// Created by fox on 9/26/24.
//

#include "ReactiveDb.h"

vector<tuple<void*, string, void*>>
ReactiveDb::query(optional<void*> subj, optional<string> pred, optional<void*> obj) {
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
