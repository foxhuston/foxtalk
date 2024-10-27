//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>


class ClockHandler : public Handler {

protected:
    void handle(const std::vector<Tuple> &queryResults) override {
        if (queryResults.empty()) return;
        if (auto subj = queryResults[0].at<uint64_t>(2)) {
            claim(Tuple {std::vector{
                TupleNoun("clock") ,
                TupleNoun("is at"),
                TupleNoun(subj.value() + 1) } });
        } else {
            throw std::runtime_error("Expected queryResults[0][2] to be a uint64_t! I GUESS IT WASN'T.");
        }
    }

    bool matches(const Tuple &n) override {
        return n.at<uint64_t>(2).has_value() &&
            n.matches<std::string>(0, "clock") &&
            n.matches<std::string>(1, "is at");
    }

    void init() override
    {
        claim(Tuple {std::vector{
            TupleNoun("clock") ,
            TupleNoun("is at"),
            TupleNoun(static_cast<uint64_t>(0))
        } });
    }
};

FOXTALK_FFI_HANDLER_REG(ClockHandler);
