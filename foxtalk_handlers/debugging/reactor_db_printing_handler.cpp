#include <foxtalk_handler.hpp>

class ReactorDbPrintingHandler : public Handler
{

protected: 
  void handle(const std::vector<Tuple> &queryResults) override
  {
    if (queryResults.size() >= 1)
    {
      claims.clear();
      auto t = queryResults[0]; 
      auto all_tuples = t.at<std::string>(2);
      if (all_tuples.has_value())
      { 
 
        std::cout << all_tuples.value() << std::endl;
        std::cout << "============================" << std::endl;
      }
    }
  }

  void init() override
  {
    claim({{{"foxtalk reactor"}, {"sees tuples"}, TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(ReactorDbPrintingHandler); 