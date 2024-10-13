//
// Created by fox on 10/12/24.
//

#include "cypher_gen.h"

#include <memory>
#include <sstream>
#include <iomanip>
#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"

using namespace kuzu::main;
namespace foxtalk::reactor::cypher_gen {

    std::string cypher_node_data(const TripleNoun& noun) {
        std::stringstream builder{};
        switch (noun.type) {
            case TripleNoun::NounType::Query:
                break;

            case TripleNoun::NounType::Symbol:
                {
                    auto quoted = std::quoted(std::get<std::string>(noun.data));
                    builder << R"({type: "Symbol", string_data: )" << quoted << "}";
                    break;
                }
            case TripleNoun::NounType::CPtr: {
                void *ptr = std::get<void *>(noun.data);
                auto ptr_location = reinterpret_cast<uint64_t>(ptr);
                builder << R"({type: "CPtr", int_data: )" << ptr_location << "}";
                break;
            }
            case TripleNoun::NounType::U64:
                builder << R"({type: "U64", int_data: )" << std::get<uint64_t>(noun.data) << "}";
                break;
            case TripleNoun::NounType::I64:
                builder << R"({type: "I64", int_data: )" << std::get<int64_t>(noun.data) << "}";
                break;
            case TripleNoun::NounType::MAX:
                // should throw? Return error?
                break;
        }
        return builder.str();
    }

    std::string store_triple_cypher(const Triple& triple) {
        std::stringstream builder{};

        /*
        MERGE (subj:Noun{type: "Symbol", string_data: "lexi"})
        MERGE (obj:Noun{type: "Symbol", string_data: "husky"})
        MERGE (subj)-[pred:Predicate{type: "Symbol", string_data: "is a"}]->(obj)
        RETURN subj, pred, obj
        */

        builder << "MERGE(subj:Noun";
        builder << cypher_node_data(triple.subject_);
        builder << ") MERGE(obj:Noun";
        builder << cypher_node_data(triple.object_);
        builder << ") MERGE (subj)-[pred:Predicate";
        builder << cypher_node_data(triple.predicate_);
        builder << "]->(obj) ";

        return builder.str();
    }

    TripleNoun triple_noun_from_kuzu_values(
            const std::vector<std::pair<std::string, std::unique_ptr<kuzu::common::Value>>> &props) {
        std::string noun_type{};
        std::optional<std::string> string_data{};
        std::optional<int64_t> int_data{};

        for (auto const &[key, val]: props) {
            if (key == "string_data" && !val->isNull()) {
                string_data = val->getValueReference<std::string>();
            }
            if (key == "int_data" && !val->isNull()) {
                int_data = val->getValue<int64_t>();
            }
            if (key == "type" && !val->isNull()) {
                noun_type = val->getValueReference<std::string>();
            }
        }

        if (noun_type == "Symbol") {
            if (string_data.has_value()) {
                return TripleNoun{string_data.value()};
            }
            throw std::runtime_error("Symbol noun type but no string data in graph.");

        }
        if (noun_type == "CPtr") {

            if (int_data.has_value()) {
                return TripleNoun{reinterpret_cast<void *>(int_data.value())};
            }
            throw std::runtime_error("CPtr noun type but no int data in graph.");

        }
        if (noun_type == "U64") {

            if (int_data.has_value()) {
                return TripleNoun{static_cast<uint64_t>(int_data.value())};
            }
            throw std::runtime_error("U64 noun type but no int data in graph.");
        }
        if (noun_type == "I64") {

            if (int_data.has_value()) {
                return TripleNoun{int_data.value()};
            }
            throw std::runtime_error("I64 noun type but no int data in graph.");

        }
        throw std::runtime_error(std::format("Noun type {} invalid", noun_type));
    }


    std::string match_for_triples_cypher(const Triple& triple) {

        std::stringstream builder{};
        builder << "MATCH(subj:Noun";
        builder << cypher_node_data(triple.subject_);
        builder << ")-[pred:Predicate";
        builder << cypher_node_data(triple.predicate_);
        builder << "]->(obj:Noun";
        builder << cypher_node_data(triple.object_);
        builder << ") ";

        return builder.str();
    }

    std::string query_for_triples_cypher(const Triple& triple) {
        std::stringstream builder{};

        builder << match_for_triples_cypher(triple);
        builder << "RETURN subj, pred, obj;";

        return builder.str();
//        auto res = connection->query(query);
        // std::cout << res->getQuerySummary();

//        std::vector<Triple> results{};

//        while (res->hasNext())
//        {
//            auto next = res->getNext();
//            auto const subj = kuzu::common::NodeVal::getProperties(next->getValue(0));
//            auto const pred = kuzu::common::RelVal::getProperties(next->getValue(1));
//            auto const obj = kuzu::common::NodeVal::getProperties(next->getValue(2));
//
//            results.emplace_back(triple_noun_from_kuzu_values(subj),
//                               triple_noun_from_kuzu_values(pred),
//                               triple_noun_from_kuzu_values(obj));
//        }
//        return results;
    }
}