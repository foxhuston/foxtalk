#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <unordered_map>

class QueryHandler : public Handler {
protected:
  void handle(const std::vector<Tuple> &queryResults) override {}
  void init() override {}

  [[nodiscard]] virtual std::string get_query() const = 0;
                virtual void handle(std::unordered_map<std::string, TupleNoun>) const = 0;

};

////////////////////////////////////////////////////////////////////////////////
// TODO: BORK

class ExampleHandler : public QueryHandler
{
  [[nodiscard]] std::string get_query() const final {
    return "(illumination /shape/@rectangle at x /x/ y /y/ width /width/ height /height/) or "
           "(illumination /shape/@circle    at x /x/ y /y/ radius /r/)";
  }

  void handle(std::unordered_map<std::string, TupleNoun> vars) const final {
    auto shape = vars["shape"].get<std::string>().value();

    if(shape == "rectangle") {
      auto x = vars["x"].get<uint64_t>().value();
      auto y = vars["y"].get<uint64_t>().value();
    } else if (shape == "circle") {
      // ...
    }

  };
};

FOXTALK_FFI_HANDLER_REG(ExampleHandler);