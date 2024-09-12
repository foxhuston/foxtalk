//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_SWAP_CHAIN_SUPPORT_DETAILS_H
#define FOXTALK_TEST_SWAP_CHAIN_SUPPORT_DETAILS_H

#include <algorithm>
#include <limits>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

struct SwapchainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;

  SwapchainSupportDetails() = delete;
  SwapchainSupportDetails(const SwapchainSupportDetails&&) = delete;

  SwapchainSupportDetails(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) {
    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    formats = physicalDevice.getSurfaceFormatsKHR(surface);
    presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  }

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat() const {
    for(const auto& fmt : formats) {
      if(fmt.format == vk::Format::eB8G8R8A8Srgb
          && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return fmt;
      }
    }

    return formats[0];
  }

  vk::PresentModeKHR chooseSwapPresentMode() const {
    auto found = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox);
    if(found != presentModes.end()) {
      return *found;
    }

    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D chooseSwapExtent(vk::Extent2D fallbackExtent) const {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      return vk::Extent2D(
        std::clamp(fallbackExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(fallbackExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
      );
    }
  }

};

#endif // FOXTALK_TEST_SWAP_CHAIN_SUPPORT_DETAILS_H
