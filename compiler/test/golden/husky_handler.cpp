#include <foxtalk_handler.hpp>

class HuskyHandler : public Handler
{
protected:
    void handle(const std::vector<Tuple> &queryResults) override
    {
        for (auto &__query_result : queryResults)
        {
            auto __who = __query_result.at<std::string>(0);
            if(!__who.has_value()) { return; }

            claim({{TupleNoun(__who.value()), TupleNoun("is"), TupleNoun("cool")}});
        }
    }

    void free_tuple(const Tuple &o) override
    {
        std::cout << "free tuple in husky handler: " << o << std::endl;
    }

    void init() override
    {
        claim({{
            TupleNoun::query(),
            TupleNoun("is"),
            TupleNoun("a"),
            TupleNoun("husky")}});
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);