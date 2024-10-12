//
// Created by fox on 10/12/24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "vendor/kuzu.hpp"

#include "foxtalk_triple.h"

namespace foxtalk::reactor::cypher_gen {

    std::string cypher_node_data(const TripleNoun& noun);

    std::string store_triple_cypher(const Triple& triple);

    TripleNoun triple_noun_from_kuzu_values(
            const std::vector<std::pair<std::string, std::unique_ptr<kuzu::common::Value>>> &props);

    std::string query_for_triples_cypher(const Triple& triple);

    std::string match_for_triples_cypher(const Triple& triple);

}