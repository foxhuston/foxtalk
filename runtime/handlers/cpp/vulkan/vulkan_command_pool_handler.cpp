// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanCommandPoolHandler : public Handler {
public:
protected:
  VkCommandPool command_pool{};
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

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // also maybe
                                                                  // protected?

        .queueFamilyIndex = 0,
    };

    vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr,
                        &command_pool);
    claim({{{command_pool}, {"is a"}, {"vulkan command pool"}}});
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
};

FOXTALK_FFI_HANDLER_REG(VulkanCommandPoolHandler);