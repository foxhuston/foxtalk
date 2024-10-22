//
// Created by lexi on 10/13/24.
//

#ifndef UPDATE_N_TUPLES_PER_TICK_H
#define UPDATE_N_TUPLES_PER_TICK_H

#include "foxtalk_handler.h"
using namespace std::literals;

class UpdateNTuplesPerTickHandler {
    int num_tuples_updated_per_tick = 0;

public:
    explicit UpdateNTuplesPerTickHandler(int n)
    {
        num_tuples_updated_per_tick = n;
    }
    FOXTALK_INIT {
        FOXTALK_REGISTER_HANDLE_QUERY({}, {"is a"s}, {"dot capturer"s});
    }

    FOXTALK_HANDLE {
        if(auto maybe_t = FOXTALK_GET_NEXT_QUERY_RESULT(); maybe_t.has_value()) {
            auto who = maybe_t->get_subject<std::string>();
            for (int i = 0; i < num_tuples_updated_per_tick; i++ )
            {

            }
            FOXTALK_CLAIM({ who.value() }, { "is"s }, { "cool"s });
        }
    }

    FOXTALK_FREE_TUPLE { }
    FOXTALK_TEARDOWN { }
};



#endif //UPDATE_N_TUPLES_PER_TICK_H
