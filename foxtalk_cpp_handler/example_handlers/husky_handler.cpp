//
// Created by fox on 10/22/24.
//

#include <foxtalk_handler.hpp>

class HuskyHandler : public Handler
{
public:
    bool matches(const Tuple& n) override
    {
        return n.matches<std::string>(1, "is a") &&
            n.matches<std::string>(2, "husky");
    }

protected:
    void handle(const std::vector<Tuple>& queryResults) override
    {
        for (auto& i : queryResults)
        {
            auto subj = i.at<std::string>(0).value();
            claim(Tuple{std::vector{TupleNoun(subj), TupleNoun("is"), TupleNoun("cool")}});
        }
    }

    void free_tuple(const Tuple& o) override
    {
        std::cout << "free tuple in husky handler: " << o << std::endl;
    }
};

static HuskyHandler* HuskyHandler_instance = nullptr;

void init()
{
    try { HuskyHandler_instance = new HuskyHandler(); }
    catch (...) { std::cerr << "CRASH in init()" << std::endl; }
}

void free_tuple()
{
    try { HuskyHandler_instance->ffi_free_tuple(_foxtalk_ipc_buffer); }
    catch (...) { std::cerr << "CRASH in free_tuple()" << std::endl; }
}

bool matches()
{
    try { return HuskyHandler_instance->ffi_matches(_foxtalk_ipc_buffer); }
    catch (...)
    {
        std::cerr << "CRASH in matches()" << std::endl;
        return false;
    }
}

void handle()
{
    try { HuskyHandler_instance->ffi_handle(_foxtalk_ipc_buffer); }
    catch (...) { std::cerr << "CRASH in handle()" << std::endl; }
}

void teardown()
{
    try { delete HuskyHandler_instance; }
    catch (...) { std::cerr << "CRASH in init()" << std::endl; }
};
