//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_QUEUE_FAMILIES_H
#define FOXTALK_TEST_QUEUE_FAMILIES_H

#include <optional>
#include <cstdint>
#include <vector>

struct QueueFamiliyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;


  bool isComplete() {
    return graphicsFamily.has_value()
      && presentFamily.has_value();
  }

  std::vector<uint32_t> indices() const {
    return std::vector<uint32_t> {
      graphicsFamily.value(), presentFamily.value()
    };
  }
};

#endif // FOXTALK_TEST_QUEUE_FAMILIES_H
