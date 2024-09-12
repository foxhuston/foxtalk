//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_QUEUE_FAMILIES_H
#define FOXTALK_TEST_QUEUE_FAMILIES_H

#include <optional>
#include <cstdint>

struct QueueFamiliyIndices {
  std::optional<uint32_t> graphicsFamily;

  bool isComplete() {
    return graphicsFamily.has_value();
  }
};

#endif // FOXTALK_TEST_QUEUE_FAMILIES_H
