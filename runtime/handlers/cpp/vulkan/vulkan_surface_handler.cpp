// pkg-config: vulkan wayland-client

#include <iostream>
#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

class VulkanSurfaceHandler : public Handler
{

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

    // glfwCreateWindowSurface(instance, window, nullptr, &surface);
    
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