#include <foxtalk_handler.hpp>
#include <fstream>

class ReactorDbPrintingHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {
    if (queryResults.size() >= 1)
    {
      claims.clear();
      auto t = queryResults[0];
      auto all_tuples = t.at<std::string>(3);
      if (all_tuples.has_value())
      {
        // debug << "Testing" << end;
        std::string filename("/tmp/foxtalk_tuples.txt");
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
    claim({{{"foxtalk reactor"}, {"has messages"}, {"listing the object db"}, TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(ReactorDbPrintingHandler);