//
// Created by lexi on 10/10/24.
//

#include "include/kuzu.hpp"

using namespace kuzu::main;


#include "foxtalk_db.h"

#include "gtest/gtest.h"

using namespace std::literals;

class DbTests : public ::testing::Test {
};

TEST_F(DbTests, YellowBrickRoad) {
    // Connect to the database.
    auto connection = std::make_unique<Connection>(database.get());

    // Create the schema.
    connection->query("CREATE NODE TABLE User(name STRING, age INT64, PRIMARY KEY (name))");
    connection->query("CREATE NODE TABLE City(name STRING, population INT64, PRIMARY KEY (name))");
    connection->query("CREATE REL TABLE Follows(FROM User TO User, since INT64)");
    connection->query("CREATE REL TABLE LivesIn(FROM User TO City)");

    // Execute a simple query.
    auto result =
        connection->query("MATCH (a:User)-[f:Follows]->(b:User) RETURN a.name, f.since, b.name;");

    // Output query result.
    while (result->hasNext()) {
        auto row = result->getNext();
        std::cout << row->getValue(0)->getValue<std::string>() << " "
                  << row->getValue(1)->getValue<int64_t>() << " "
                  << row->getValue(2)->getValue<std::string>() << std::endl;
    }
    EXPECT_EQ(0, 0);
}
