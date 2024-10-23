//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>


class HuskyHandler : public Handler {
public:
    bool matches(const Tuple &n) override {
        return n.at<std::string>(1).has_value() && n.at<std::string>(1).value() == "is a" &&
            n.at<std::string>(2).has_value() && n.at<std::string>(2).value() == "husky";
    }

protected:
    void handle(const std::vector<Tuple> &queryResults) override {
        for (auto& i: queryResults)
        {
            auto subj = i.at<std::string>(0).value();
            claim(Tuple {std::vector{TupleNoun(subj) , TupleNoun("is"), TupleNoun("cool") } });
        }
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);
