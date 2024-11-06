//
// Created by fox on 10/22/24.
//

#include <ctime>
#include <cstdint>
#include <foxtalk_handler.hpp>

class ClockHandler : public Handler {
private:
  uint64_t last_time = 0;

  // 1/60s
  static constexpr uint64_t tick_rate_ns = 1.667e7;
  static constexpr double ONE_NS = 1e9;

public:
  void handle(const std::vector<Tuple> &queryResults) override {
    struct timespec t {};
    clock_gettime(CLOCK_REALTIME, &t);

    // std::cout << "RTC HANDLE: " << t.tv_nsec << std::endl;
    // double delta = (t.tv_nsec - last_time) / ONE_NS;

    claim({{ {"clock"}, {"ns"}, {t.tv_nsec}, {"delta"}, {t.tv_nsec - last_time} }});

    last_time = t.tv_nsec;
  }

  bool poll() override {
    struct timespec t {};
    clock_gettime(CLOCK_REALTIME, &t);
    auto now = t.tv_nsec;

    return (now - last_time) > tick_rate_ns;
  }

  void init() override {
    claim({{{"foxtalk"}, {"has"}, {"started"}}});
  }

  void register_initial_tuples() override {
    struct timespec t {};
    clock_gettime(CLOCK_REALTIME, &t);

    last_time = t.tv_nsec;

    claim({{ {"clock"}, {"ns"}, {t.tv_nsec}, {"delta"}, {0ul} }});
  }
};

FOXTALK_FFI_HANDLER_REG(ClockHandler);
