// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanSwapchainBufferHandler : public Handler {
  const int MAX_NUM_IMAGES = 4;

public:
protected:
  VkSwapchainKHR swapchain{};
  void handle(const std::vector<Tuple> &queryResults) override {
    if(queryResults.size() != 1) { return; }
    auto q = queryResults[0];
    
    std::cout << "[SwapchainBufferHandler] found the swapchain!" << std::endl;
    
    auto swapchain = static_cast<VkSwapchainKHR>(q.at<void*>(0).value());
    for(int i = 4; i < q.size(); i++) {
      auto img = static_cast<VkImage>(q.at<void*>(i).value());
      std::cout << "Found image: " << img << std::endl;
    }
  }

  void init() override {
    claim({{
      TupleNoun::query(),
      {"is a"}, {"vulkan swapchain"},
      {"with images"},
      TupleNoun::prefix()
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainBufferHandler);