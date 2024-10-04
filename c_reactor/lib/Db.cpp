//
// Created by fox on 10/1/24.
//

#include <format>
#include <iostream>
#include "TupleNoun.h"
#include "Db.h"

namespace foxtalk {
    void Db::inc_tuple_noun(const TupleNoun *tn) {
        if(_tuplenoun_refcount.contains(tn)) {
            _tuplenoun_refcount.at(tn)++;
        } else {
            _tuplenoun_refcount.insert({ tn, 1 });
        }

//        std::cout << "INCREF! Refcount for " << *tn << " is " << _tuplenoun_refcount.at(tn) << std::endl;
    }

    void Db::dec_tuple_noun(const TupleNoun *tn) {
        if(_tuplenoun_refcount.contains(tn)) {
            auto count = --_tuplenoun_refcount.at(tn);

//            std::cout << "DECREF! Refcount for " << *tn << " is " << count << std::endl;
            if(count == 0) {
//                std::cout << "    Deleting!" << std::endl;
                _tuplenoun_refcount.erase(tn);
                delete tn;
            } else if (count < 0) {
                throw std::runtime_error("Got a negative refcount for tuple!");
            }
        }
    }

    void Db::add_tuple_nouns(const Tuple *tuple) {
        inc_tuple_noun(tuple->getSubject());
        inc_tuple_noun(tuple->getPredicate());
        inc_tuple_noun(tuple->getObject());
    }

    void Db::remove_tuple_nouns(const Tuple *tuple) {
        dec_tuple_noun(tuple->getSubject());
        dec_tuple_noun(tuple->getPredicate());
        dec_tuple_noun(tuple->getObject());
    }

    void Db::add_tuple(const Tuple *tuple) {
        add_tuple_nouns(tuple);
        _all_tuples.insert(tuple);
    }

    void Db::remove_tuple(const Tuple *tuple) {
        const Tuple *actual_element = nullptr;
        add_tuple_nouns(tuple);

        if(auto it = _all_tuples.find(tuple); it != _all_tuples.end() && *it != tuple) {
//            std::cout << "Found a different element in db!" << std::endl;
            actual_element = *it;
        }

        _all_tuples.erase(tuple);

        if(actual_element) {
            remove_tuple_nouns(actual_element);
        }

        remove_tuple_nouns(tuple);

        if(actual_element) {
            delete actual_element;
        }

        delete tuple;
    }

    // This copies the whole hash set??????
    const ReactorSet<const Tuple*>::type& Db::get_tuples() const {
        return _all_tuples;
    }

} // foxtalk