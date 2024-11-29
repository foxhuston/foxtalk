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
  std::vector<VkCommandBuffer> command_buffers{};
  void handle(const std::vector<Tuple> &queryResults) override {

    auto frames_in_flight_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(3) == "frames in flight";
        });

    if (frames_in_flight_tuple == queryResults.end()) {
      err << "Query results did not include the number of frames in flight"
          << end;
      return;
    }

    auto frames_in_flight = frames_in_flight_tuple->at<uint64_t>(2).value();
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

    auto queue_family_index = logical_device_tuple->at<uint64_t>(6).value();

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // also maybe
                                                                  // protected?

        .queueFamilyIndex = (uint32_t)queue_family_index,
    };

    auto result = vkCreateCommandPool(logical_device, &command_pool_create_info,
                                      nullptr, &command_pool);

    if (result != VK_SUCCESS) {
      err << "Could not create command pool" << end;
      return;
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = frames_in_flight;
    command_buffers.resize(frames_in_flight);
    debug << "Creating " << frames_in_flight << " command buffers..." << end;

    if (vkAllocateCommandBuffers(logical_device, &allocInfo,
                                 command_buffers.data()) != VK_SUCCESS) {
      err << "Could not create command buffers" << end;
      return;
    }
    for (uint64_t i = 0; i < command_buffers.size(); i++) {
      claim({{{command_pool},
              {"is a"},
              {"vulkan command pool"},
              {"for device"},
              {logical_device},
              {"with command buffer index"},
              {i},
              {command_buffers[i]}}});
    }
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
            TupleNoun::prefix()}});

    claim({{{"vulkan"},
            {"should have"},
            TupleNoun::query(),
            {"frames in flight"}}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan command pool"))) {

      auto cmd_pool = static_cast<VkCommandPool>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());
      debug << "Freeing command pool " << t << end;
      vkDestroyCommandPool(logical_device, cmd_pool, nullptr);
    } 
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanCommandPoolHandler);