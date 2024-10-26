//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>

class HuskyHandler : public Handler
{
public:
    bool matches(const Tuple &n) override
    {
        return n.matches<std::string>(1, "is a") &&
               n.matches<std::string>(2, "husky");
    }

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
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);