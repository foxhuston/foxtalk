////
//// Created by fox on 10/22/24.
////
//
//#include <foxtalk_handler.hpp>
//
//class AggHandler : public Handler {
//public:
//    bool matches(const Tuple &n) override {
//        return n.at<uint64_t>(0).has_value();
//    }
//
//protected:
//    void handle(const std::vector<Tuple> &queryResults) override {
//        uint64_t sum = 0;
//
//        for (auto& i: queryResults)
//        {
//            auto subj = i.at<uint64_t>(0).value();
//            sum += subj;
//        }
//        claim(Tuple {std::vector{TupleNoun("all number subjects") , TupleNoun("sum to"), TupleNoun(sum) } });
//    }
//};
//
//FOXTALK_FFI_HANDLER_REG(AggHandler);
