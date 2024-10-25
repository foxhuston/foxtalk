//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>


class HuskyHandler : public Handler {
public:
    bool matches(const Tuple &n) override {
        return n.at<uint64_t>(0).has_value();
    }

protected:
    void handle(const std::vector<Tuple> &queryResults) override {
        uint64_t sum = 0;

        for (auto& i: queryResults)
        {
            auto subj = i.at<std::string>(0).value();
            claim(Tuple {std::vector{TupleNoun(subj) , TupleNoun("is"), TupleNoun("cool") } });
        }
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);
