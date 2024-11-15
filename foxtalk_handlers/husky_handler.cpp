//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>

class HuskyHandler : public Handler
{
protected:
    void handle(const std::vector<Tuple> &queryResults) override
    {
        for (auto &i : queryResults)
        {
            auto subj = i.at<std::string>(0).value();
            claim(Tuple{std::vector{TupleNoun(subj), TupleNoun("is"), TupleNoun("cool")}});
        }
    }

    void free_tuple(const Tuple &o) override
    {

        std::cout << "free tuple in husky handler: " << o << std::endl;
    }

    void init() override
    {
        std::cout << "Init in husky handler! 33333" << std::endl;
        claim(Tuple{std::vector{
            TupleNoun::query(),
            TupleNoun("is a"),
            TupleNoun("husky")}});
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);