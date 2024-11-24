// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanSyncObjectsHandler : public Handler {
public:
protected:
  VkSemaphore semaphore{};
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 1) {
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

    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr,
                        &semaphore);
    claim({{{semaphore}, {"is a"}, {"vulkan semaphore"}, {"for device"}, {logical_device}}});
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
  }
  

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan semaphore"))) {
      
      auto semaphore = static_cast<VkSemaphore>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());

      vkDestroySemaphore(logical_device, semaphore, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSyncObjectsHandler);