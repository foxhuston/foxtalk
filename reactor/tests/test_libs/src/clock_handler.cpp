//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>


class ClockHandler : public Handler {
public:
    bool matches(const Tuple &n) override {
        return n.at<uint64_t>(2).has_value() &&
            n.matches<std::string>(0, "clock") &&
            n.matches<std::string>(1, "is at");
    }

protected:
    void handle(const std::vector<Tuple> &queryResults) override {
        auto subj = queryResults[0].at<uint64_t>(0).value();
        claim(Tuple {std::vector{TupleNoun("clock") , TupleNoun("is at"), TupleNoun(subj + 1) } });
    }
};

FOXTALK_FFI_HANDLER_REG(ClockHandler);
