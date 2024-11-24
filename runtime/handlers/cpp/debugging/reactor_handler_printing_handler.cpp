#include <foxtalk_handler.hpp>
#include <fstream>

class ReactorHandlerPrintingHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {
    if (queryResults.size() >= 1)
    {
      claims.clear();
      auto t = queryResults[0];
      // log_debug(t << "----");
      auto all_tuples = t.at<std::string>(3);
      if (all_tuples.has_value())
      {
        // log_debug("Testing");
        std::string filename("/tmp/foxtalk_handlers.txt");
        std::ofstream stream (filename);
        if (!stream.is_open()) {
          std::cerr << "Can't open file " << filename << std::endl; 
          return;
        }
        stream << all_tuples.value() << std::endl;
      }
    }
  }

  void init() override
  {
    claim({{{"foxtalk reactor"}, {"has messages"}, {"listing all handlers"}, TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(ReactorHandlerPrintingHandler);