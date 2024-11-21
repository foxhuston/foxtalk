// pkg-config: vulkan wayland-client

#include <iostream>
#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include "wayland-client.h"

#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

class VulkanSurfaceHandler : public Handler
{

  std::vector<std::string> extensions = {
    "VK_KHR_surface",
    "VK_KHR_wayland_surface",
  };

protected:
  void handle(const std::vector<Tuple> &queryResults) override {

    // std::cout << "Hello" << std::endl;
    if (queryResults.size() != 2) { return; }

    auto instance_tuple = std::find_if(
      queryResults.begin(),
      queryResults.end(),
      [](const Tuple& result) {
        return result.at<std::string>(2) == "vulkan instance";
      });

    if (instance_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan instance" << std::endl;
      return;
    }

    VkInstance instance = static_cast<VkInstance>(instance_tuple->at<void *>(0).value());


    VkSurfaceKHR surface {};
    
    
    // VkWaylandSurfaceCreateInfoKHR creationInfo{
    //   .sType = VkStructureType::VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    //   .pNext = nullptr,
    //   .flags = 0,
    //   .display = display,
    //   .surface = surface
    // };

    // auto surface_ret = vkCreateWaylandSurfaceKHR(instance, &creationInfo, nullptr, &surface);
    // std::cout << surface_ret << std::endl;
    // std::cout << "Wayland display connected" << std::endl;
  }

  void free_tuple(const Tuple &t) override {
    // auto display = static_cast<wl_display*>(t.at<void *>(0).value());
    // wl_display_disconnect(display);
    // std::cout << "Disconnecting wayland display" << std::endl;
  }

  void init() override {
    
    
    claim({{{TupleNoun::query()}, {"is the"}, {"vulkan instance"}}});

    claim({{
      TupleNoun::query(),
      {"is the"},
      {"vulkan logical device"},
      {"with graphics queue"},
      TupleNoun::query(),
      {"with queue family index"},
      TupleNoun::query(),
      TupleNoun::prefix()
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSurfaceHandler);