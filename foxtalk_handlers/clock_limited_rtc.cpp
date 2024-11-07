//
// Created by fox on 10/22/24.
//

#include <ctime>
#include <cstdint>
#include <foxtalk_handler.hpp>

class ClockHandler : public Handler {
private:
  double last_time = 0;

  // 1/60s
  static constexpr double tick_rate_ns = 1.0/60;
  static constexpr double ONE_NS = 1e9;

  double get_time_as_double() {
    struct timespec t {};
    clock_gettime(CLOCK_REALTIME, &t);
    return (double)t.tv_sec + ((double)t.tv_nsec / 1E7);  
  }

public:
  void handle(const std::vector<Tuple> &queryResults) override {
    
    auto now = get_time_as_double();
    double delta = (now - last_time);

    // std::cout << "RTC HANDLE: " << delta << std::endl;

    claim({{ {"clock"}, {"epoch time"}, {now}, {"delta"}, {delta} }});

    last_time = now; 
  }

  bool poll() override {
    auto now = get_time_as_double();

    return (now - last_time) > tick_rate_ns; 
  }

  void init() override {
    claim({{{"foxtalk"}, {"has"}, {"started"}}});
  }

  void register_initial_tuples() override {
    last_time = get_time_as_double();

    claim({{ {"clock"}, {"epoch time"}, {last_time}, {"delta"}, {0ul} }});
  }
};

FOXTALK_FFI_HANDLER_REG(ClockHandler);
