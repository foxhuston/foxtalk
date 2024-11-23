// pkg-config: vulkan wayland-client

#include "foxtalk_tuple.h"
#include "wayland-client.h"
#include <foxtalk_handler.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

class VulkanWaylandSurfaceHandler : public Handler {

  std::vector<std::string> extensions = {
      "VK_KHR_surface",
      "VK_KHR_wayland_surface",
  };

  VkSurfaceKHR *khr_surface = new VkSurfaceKHR{};

protected:

  // stolen from vkcube project
  // https://github.com/krh/vkcube/blob/master/main.c#L218
  std::optional<VkFormat> choose_surface_format(VkPhysicalDevice physical_device) {
    uint32_t num_formats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *khr_surface,
                                         &num_formats, nullptr);
    
    assert(num_formats > 0);

    std::vector<VkSurfaceFormatKHR> formats (num_formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *khr_surface,
                                         &num_formats, formats.data());

    for (const auto& format: formats ) {
      // std::cout << "supported px format: " << format.format << " with color space " << format.colorSpace << std::endl;
      switch (format.format) {
      case VK_FORMAT_R8G8B8A8_SRGB:
        return format.format;
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
        /* We would like to support these but they don't seem to work. */
      default:
        continue;
      }
    }  
    std::cerr << "Could not choose a pixel format given the created khr surface" << std::endl;
    return std::nullopt;
  
  }

  void handle(const std::vector<Tuple> &queryResults) override {
    std::cout << "Wayland Surface Handler Called with " << queryResults.size()
              << " query result(s)." << std::endl;

    // std::cout << "Hello" << std::endl;
    if (queryResults.size() != 3) {
      return;
    }

    auto instance_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan instance";
        });

    if (instance_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan instance"
                << std::endl;
      return;
    }

    VkInstance instance =
        static_cast<VkInstance>(instance_tuple->at<void *>(0).value());

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

    auto wayland_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "wayland display";
        });

    if (wayland_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the wayland display"
                << std::endl;
      return;
    }

    wl_display *display =
        static_cast<wl_display *>(wayland_tuple->at<void *>(0).value());
    wl_surface *surface =
        static_cast<wl_surface *>(wayland_tuple->at<void *>(4).value());

    // std::cout << "Found all three tuples" << std::endl;
    auto get_wayland_presentation_support =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)
            vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    auto create_wayland_surface =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(
            instance, "vkCreateWaylandSurfaceKHR");

    if (!get_wayland_presentation_support(physical_device, 0, display)) {
      std::cerr << "Vulkan not supported on given Wayland surface" << std::endl;
      return;
    }

    VkWaylandSurfaceCreateInfoKHR surface_create_info{
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = display,
        .surface = surface,
    };

    create_wayland_surface(instance, &surface_create_info, nullptr,
                           khr_surface);

    // std::cout << "Wayland supported and surface created!" << std::endl;

    if (auto maybe_chosen_surface_format = choose_surface_format(physical_device)) {
      claim({{
        {khr_surface},
        {"is a"},
        {"vk surface khr"},
        {"with surface pixel format value"},
        {(uint64_t)maybe_chosen_surface_format.value()}
        }});
        return;
    }
    std::cerr << "Could not create a vk surface khr from the wayland surface handler!" << std::endl;

  }

  void free_tuple(const Tuple &t) override {
    // auto display = static_cast<wl_display*>(t.at<void *>(0).value());
    // wl_display_disconnect(display);
    // std::cout << "Disconnecting wayland display" << std::endl;
  }

  void init() override {

    claim({{{TupleNoun::query()}, {"is the"}, {"vulkan instance"}}});

    claim(
        {{TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"wayland display"},
            {"with wl_surface"},
            TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanWaylandSurfaceHandler);