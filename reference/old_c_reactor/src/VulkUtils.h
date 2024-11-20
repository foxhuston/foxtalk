//
// Created by fox on 10/3/24.
//

#ifndef FOXTALK_VULKUTILS_H
#define FOXTALK_VULKUTILS_H

#include <vulkan/vulkan.hpp>
#include <cstdint>

static uint32_t findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
  auto memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

#endif //FOXTALK_VULKUTILS_H
