//
// Created by lexi on 10/10/24.
//

#ifndef FOXTALK_DB_H
#define FOXTALK_DB_H
#include <foxtalk_triple.h>
#include <memory>
#include <sstream>

#include "vendor/kuzu.hpp"

using namespace kuzu::main;
// #include <format>

class foxtalk_db
{
private:
    std::unique_ptr<Database> database;

public:
    explicit foxtalk_db(SystemConfig config)
    {
        database = std::make_unique<Database>(":memory:", config);

        auto connection = std::make_unique<Connection>(database.get());

        auto res = connection->query("CREATE NODE TABLE Noun (type STRING, string_data STRING, int_data INT64, id SERIAL, PRIMARY KEY (id))");
        // assert(res->isSuccess());
        res = connection->query("CREATE REL TABLE Predicate(FROM Noun TO Noun, type STRING, string_data STRING, int_data INT64)");
        // assert(res->isSuccess());
    }

    static std::string cypher_node_data(const TripleNoun *noun)
    {
        std::stringstream builder{};
        switch (noun->type)
        {
        case TripleNoun::NounType::Query:
            break;
        case TripleNoun::NounType::Symbol:
            builder << R"({type: "Symbol", string_data: ")" << std::get<std::string>(noun->data) << "\"}";
            break;
        case TripleNoun::NounType::CPtr:
        {
            void *ptr = std::get<void *>(noun->data);
            auto ptr_location = reinterpret_cast<uint64_t>(ptr);
            builder << R"({type: "CPtr", int_data: )" << ptr_location << "}";
            break;
        }
        case TripleNoun::NounType::U64:
            builder << R"({type: "U64", int_data: )" << std::get<uint64_t>(noun->data) << "}";
            break;
        case TripleNoun::NounType::I64:
            builder << R"({type: "I64", int_data: )" << std::get<int64_t>(noun->data) << "}";
            break;
        case TripleNoun::NounType::MAX:
            // should throw? Return error?
            break;
        }
        return builder.str();
    }

    void store_triple(Triple *triple)
    {
        auto connection = std::make_unique<Connection>(database.get());

        std::stringstream builder{};

        builder << "MERGE(:Noun";
        builder << cypher_node_data(&triple->subject_);
        builder << ")-[:Predicate";
        builder << cypher_node_data(&triple->predicate_);
        builder << "]->(:Noun";
        builder << cypher_node_data(&triple->object_);
        builder << ");";

        auto query = builder.str();
        auto res = connection->query(query);
        // assert(res->isSuccess());
        // std::cout << res->getQuerySummary();
    }

    static TripleNoun triple_noun_from_kuzu_values(const std::vector<std::pair<std::string, std::unique_ptr<kuzu::common::Value>>>& props)
    {
        std::string noun_type {};
        std::optional<std::string> string_data {};
        std::optional<int> int_data {};

        for (auto const& [key, val]: props)
        {
            if (key == "string_data" && !val->isNull())
            {
                string_data = val->getValueReference<std::string>();
            }
            if (key == "int_data" && !val->isNull())
            {
                int_data = val->getValue<int64_t>();
            }
            if (key == "type" && !val->isNull())
            {
                noun_type = val->getValueReference<std::string>();
            }
        }

        if (noun_type == "Symbol")
        {
            if (string_data.has_value())
            {
                return TripleNoun{string_data.value()};
            }
            throw std::runtime_error("Symbol noun type but no string data in graph.");

        }
        if (noun_type == "CPtr")
        {

            if (int_data.has_value())
            {
                return TripleNoun{reinterpret_cast<void *>(int_data.value())};
            }
            throw std::runtime_error("CPtr noun type but no int data in graph.");

        }
        if (noun_type == "U64")
        {

            if (int_data.has_value())
            {
                return TripleNoun{static_cast<uint64_t>(int_data.value())};
            }
            throw std::runtime_error("U64 noun type but no int data in graph.");
        }
        if (noun_type == "I64")
        {

            if (int_data.has_value())
            {
                return TripleNoun{int_data.value()};
            }
            throw std::runtime_error("I64 noun type but no int data in graph.");

        }
        throw std::runtime_error(std::format("Noun type {} invalid", noun_type));
    }

    std::vector<Triple> get_triples(const Triple *triple)
    {
        auto connection = std::make_unique<Connection>(database.get());

        std::stringstream builder{};
        builder << "MATCH(subj:Noun";
        builder << cypher_node_data(&triple->subject_);
        builder << ")-[pred:Predicate";
        builder << cypher_node_data(&triple->predicate_);
        builder << "]->(obj:Noun";
        builder << cypher_node_data(&triple->object_);
        builder << ") ";
        builder << "RETURN subj, pred, obj;";

        auto query = builder.str();
        auto res = connection->query(query);
        // std::cout << res->getQuerySummary();

        std::vector<Triple> results{};

        while (res->hasNext())
        {
            auto next = res->getNext();
            auto const subj = kuzu::common::NodeVal::getProperties(next->getValue(0));
            auto const pred = kuzu::common::RelVal::getProperties(next->getValue(1));
            auto const obj = kuzu::common::NodeVal::getProperties(next->getValue(2));

            results.emplace_back(triple_noun_from_kuzu_values(subj),
                               triple_noun_from_kuzu_values(pred),
                               triple_noun_from_kuzu_values(obj));
        }
        return results;
    }
};

#endif // FOXTALK_DB_H
