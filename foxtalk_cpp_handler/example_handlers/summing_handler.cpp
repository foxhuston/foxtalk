//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>

FOXTALK_FFI_HANDLER(SummingHandler, query_results) {

}

FOXTALK_HANDLER_MATCHES(SummingHandler, tup) {
    return tup.at<uint64_t>(0).has_value();
}

