// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanSwapchainHandler : public Handler {
  const int MAX_NUM_IMAGES = 4;

public:
protected:
  VkSwapchainKHR swapchain{};
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 3) {
      return;
    }

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan logical device"
                << std::endl;
      return;
    }

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto queue_family_index = logical_device_tuple->at<uint64_t>(6).value();

    auto surface_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vk surface khr";
        });

    if (surface_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vk surface khr"
                << std::endl;
      return;
    }

    auto surface =
        static_cast<VkSurfaceKHR>(surface_tuple->at<void *>(0).value());

    auto pixel_format =
        static_cast<VkFormat>(surface_tuple->at<uint64_t>(4).value());

    auto physical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "chosen vulkan physical device";
        });

    if (physical_device_tuple == queryResults.end()) {
      std::cerr
          << "Query results did not include the chosen vulkan physical device"
          << std::endl;
      return;
    }

    VkPhysicalDevice physical_device = static_cast<VkPhysicalDevice>(
        physical_device_tuple->at<void *>(0).value());

    VkSurfaceCapabilitiesKHR surface_caps{};

    // std::cout << "[Swapchain] Physical device is: " << physical_device <<
    // std::endl; std::cout << "[Swapchain] Surface is: " << surface <<
    // std::endl;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                              &surface_caps);
    assert(surface_caps.supportedCompositeAlpha &
           VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface,
                                         &supported);
    assert(supported);

    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count,
                                              nullptr);
    std::vector<VkPresentModeKHR> present_modes{count};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count,
                                              present_modes.data());
    int i;
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (i = 0; i < count; i++) {
      if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
        present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
      }
    }

    uint32_t minImageCount = 2;
    if (minImageCount < surface_caps.minImageCount) {
      if (surface_caps.minImageCount > MAX_NUM_IMAGES) {

        std::cerr << "surface_caps.minImageCount is too large (is: "
                  << surface_caps.minImageCount << ", max: " << MAX_NUM_IMAGES
                  << std::endl;
        return;
      }
      minImageCount = surface_caps.minImageCount;
    }

    if (surface_caps.maxImageCount > 0 &&
        minImageCount > surface_caps.maxImageCount) {
      minImageCount = surface_caps.maxImageCount;
    }
    std::vector<uint32_t> queue_family_indices{};
    queue_family_indices.push_back(queue_family_index);
    VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .flags = 0,
        .surface = surface,
        .minImageCount = minImageCount,
        .imageFormat = pixel_format,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = {500, 500},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1, // Probably want to query on this
        .pQueueFamilyIndices = queue_family_indices.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
    };
    auto swapchainCreationResult = vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swapchain);
    if(swapchainCreationResult != VK_SUCCESS) {
      std::cerr << "Error creating swapchain!" << std::endl;
      return;
    }


    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, nullptr);
    if (image_count == 0) {
      std::cerr << "[Vulkan Swapchain Handler] Image count from the 'get "
                   "swapchain' call returned 0;" << std::endl;
      return;
    }
    std::vector<VkImage> swapchain_images{image_count};
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count,
                            swapchain_images.data());

    if (image_count > MAX_NUM_IMAGES) {
      std::cerr << "[Vulkan Swapchain Handler] Image count from the 'get "
                   "swapchain' call returned "
                << image_count << ", which is higher than max num images "
                << MAX_NUM_IMAGES << std::endl;
      return;
    }
    
    std::cout << "[Swapchain Handler] found " << swapchain_images.size() << " swapchain images" << std::endl;

    std::vector<TupleNoun> imageNouns = {
      {swapchain}, {"is a"}, {"vulkan swapchain"},
      {"with images"}
    };
    
    for(auto* image : swapchain_images) {
      imageNouns.emplace_back(image);
    }

    claim(Tuple {std::move(imageNouns)});
  }

  void init() override {
    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        {"with graphics queue"},
        TupleNoun::query(),
        {"with queue family index"},
        TupleNoun::query(),
    }});
    claim(
        {{TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vk surface khr"},
            {"with surface pixel format value"},
            TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainHandler);