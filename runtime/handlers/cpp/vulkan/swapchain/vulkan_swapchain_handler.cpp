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
  std::vector<TupleNoun> last_created_swapchain_nouns{};
  void handle(const std::vector<Tuple> &queryResults) override {

    // std::cout << "In handle of VulkanSwapchainHandler" << std::endl;

    auto last_out_of_date = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(3) == "is out of date";
        });

    auto need_to_create_new_swapchain = false;
    if (last_created_swapchain_nouns.size() < 5) {
      // std::cout << "Creating a new swapchain for the first time..."
      //           << std::endl;
      need_to_create_new_swapchain = true;
    } else if (last_out_of_date != queryResults.end()) {
      auto ood_swapchain_version = last_out_of_date->at<double_t>(2).value();
      if (ood_swapchain_version ==
          last_created_swapchain_nouns[4].get<double_t>().value()) {
        debug
            << "[SwapchainHandler] Need to create a new swapchain because the last one, version "
            << ood_swapchain_version << ", was out of date" << end;
        need_to_create_new_swapchain = true;
      }
    }

    auto surface_extent_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(0) == "available surface has width";
        });

    if (surface_extent_tuple == queryResults.end()) {
      err << "Query results did not include the available surface extent"
          << end;
      return;
    }

    auto width = surface_extent_tuple->at<uint64_t>(1).value();
    auto height = surface_extent_tuple->at<uint64_t>(3).value();

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      err << "Query results did not include the vulkan logical device" << end;
      return;
    }

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto graphics_queue =
        static_cast<VkQueue>(logical_device_tuple->at<void *>(4).value());
    auto graphics_queue_index = logical_device_tuple->at<uint64_t>(6).value();
    auto present_queue =
        static_cast<VkQueue>(logical_device_tuple->at<void *>(8).value());
    auto present_queue_index = logical_device_tuple->at<uint64_t>(10).value();

    auto surface_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vk surface khr";
        });

    if (surface_tuple == queryResults.end()) {
      err << "Query results did not include the vk surface khr" << end;
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
      err << "Query results did not include the chosen vulkan physical device"
          << end;
      return;
    }
    debug << "Gonna create a new swapchain? " << need_to_create_new_swapchain << end;
    if (need_to_create_new_swapchain) {

      VkPhysicalDevice physical_device = static_cast<VkPhysicalDevice>(
          physical_device_tuple->at<void *>(0).value());

      VkSurfaceCapabilitiesKHR surface_caps{};
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                                &surface_caps);
      if ((surface_caps.supportedCompositeAlpha &
           VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0) {
        err << "surface capability did not support composite alpha "
               "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR"
            << end;
      }

      VkBool32 supported;
      vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface,
                                           &supported);
      if (!supported) {
        err << "vkGetPhysicalDeviceSurfaceSupportKHR returned false for this "
               "surface"
            << end;
      }

      uint32_t count;
      vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                                &count, nullptr);
      std::vector<VkPresentModeKHR> present_modes{count};
      vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                                &count, present_modes.data());
      int i;
      VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      uint32_t minImageCount = 2;
      if (minImageCount < surface_caps.minImageCount) {
        if (surface_caps.minImageCount > MAX_NUM_IMAGES) {

          err << "surface_caps.minImageCount is too large (is: "
              << surface_caps.minImageCount << ", max: " << MAX_NUM_IMAGES
              << end;

          return;
        }
        minImageCount = surface_caps.minImageCount;
      }

      if (surface_caps.maxImageCount > 0 &&
          minImageCount > surface_caps.maxImageCount) {
        minImageCount = surface_caps.maxImageCount;
      }

      std::vector<uint32_t> queue_family_indices{(uint32_t)graphics_queue_index,
                                                 (uint32_t)present_queue_index};
      std::cout << "Swapchain being created is ... " << width << "x" << height
                << std::endl;
      VkSwapchainCreateInfoKHR create_info{
          .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
          .flags = 0,
          .surface = surface,
          .minImageCount = minImageCount,
          .imageFormat = pixel_format,
          .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
          .imageExtent = {(uint32_t)width, (uint32_t)height},
          .imageArrayLayers = 1,
          .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
          .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
          .presentMode = present_mode,
      };

      if (graphics_queue_index != present_queue_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
      } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      }
      if (swapchain != nullptr) {
        create_info.oldSwapchain = swapchain;
      }
      auto swapchainCreationResult = vkCreateSwapchainKHR(
          logical_device, &create_info, nullptr, &swapchain);
      if  (swapchainCreationResult == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
        debug << "Tried to create a new swapchain, but the surface hasn't been recreated yet... so, just resserting the old one for now" << end;
        reuse_swapchain();
        return;
      }
      if (swapchainCreationResult != VK_SUCCESS) {
        err << "Error creating swapchain! Result: " << swapchainCreationResult << end;
        return;
      }

      uint32_t image_count = 0;
      vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, nullptr);
      if (image_count == 0) {
        err << "Image count from the 'get "
               "swapchain' call returned 0"
            << end;
        return;
      }
      std::vector<VkImage> swapchain_images{image_count};
      vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count,
                              swapchain_images.data());

      if (image_count > MAX_NUM_IMAGES) {
        err << "Image count from the 'get swapchain' call returned "
            << image_count << ", which is higher than max num images "
            << MAX_NUM_IMAGES << end;
        return;
      }

      debug << "found" << swapchain_images.size() << " swapchain images" << end;
      auto version = get_time_as_double();
      std::vector<TupleNoun> imageNouns = {{swapchain},
                                           {"is a"},
                                           {"vulkan swapchain"},
                                           {"at version"},
                                           {version},
                                           {"for device"},
                                           {logical_device},
                                           {"with pixel format value"},
                                           {(uint64_t)pixel_format},
                                           {"with images"}};

      for (auto *image : swapchain_images) {
        imageNouns.emplace_back(image);
      }

      std::vector<TupleNoun> tmp;
      std::copy(imageNouns.begin(), imageNouns.end(), std::back_inserter(tmp));

      last_created_swapchain_nouns = std::move(tmp);
      std::cout << "Created new swapchain with version " << version << std::endl;
      claim(std::move(Tuple{std::move(imageNouns)}));
    } else {
      reuse_swapchain();
    }
  }

  void reuse_swapchain() {
      std::vector<TupleNoun> tmp;
      std::copy(last_created_swapchain_nouns.begin(),
                last_created_swapchain_nouns.end(), std::back_inserter(tmp));
      debug << "Reusing swapchain with version " << last_created_swapchain_nouns[4].get<double_t>().value() << end;
      claim(std::move(Tuple{std::move(tmp)}));
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
    claim(
        {{TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}}});
    claim({{{"swapchain"},
            {"at version"},
            TupleNoun::query(),
            {"is out of date"}}});

    claim({{
        {"available surface has width"},
        TupleNoun::query(),
        {"and height"},
        TupleNoun::prefix(),
    }});
    claim({{TupleNoun::query(),
            {"is a"},
            {"vk surface khr"},
            {"with surface pixel format value"},
            TupleNoun::query(),
            TupleNoun::prefix()}});
  }
  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan swapchain"))) {

      debug << "Freeing swapchain" << t << end;

      auto swapchain = static_cast<VkSwapchainKHR>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(6).value());
      debug << "Freeing swapchain " << t << end;
      // vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
    } 
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainHandler);