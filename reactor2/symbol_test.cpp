#include <cstring>
#include "gtest/gtest.h"

#include "symbol.h"

class SymbolTests : public ::testing::Test {

};

TEST_F(SymbolTests, SymbolInternIsIdempotent) {
    auto symA = intern("hello");
    auto symB = intern("hello");

    EXPECT_EQ(symA, symB);
}

TEST_F(SymbolTests, DifferentSymbolsAreDifferent1) {
    auto symA = intern("hello");
    auto symB = intern("hel");

    EXPECT_NE(symA, symB);
}

TEST_F(SymbolTests, SymbolsStoreTheirCStringReprs) {
    auto hello = intern("hello");
    auto hell = intern("hell");

    EXPECT_STREQ(hello->str, "hello");
    EXPECT_STREQ(hell->str, "hell");
}
