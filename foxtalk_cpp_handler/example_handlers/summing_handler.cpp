//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>


class SummingHandler : public Handler {
public:
    bool matches(const Tuple &n) override {
        return false;
    }

protected:
    void handle(const std::vector<Tuple> &queryResults) override {

    }
};

FOXTALK_FFI_HANDLER_REG(SummingHandler);
