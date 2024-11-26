// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanSyncObjectsHandler : public Handler {
public:
protected:
  VkSemaphore image_available_semaphore{};
  VkSemaphore render_finished_semaphore{};
  VkFence in_flight_fence{};

  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 2) {
      return;
    }

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

    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    auto result = vkCreateSemaphore(logical_device, &semaphore_create_info,
                                    nullptr, &image_available_semaphore);
    result = vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr,
                               &render_finished_semaphore);
    claim({{{image_available_semaphore},
            {"is a"},
            {"vulkan semaphore"},
            {"for device"},
            {logical_device},
            {"signaling image is available"}}});
    claim({{{render_finished_semaphore},
            {"is a"},
            {"vulkan semaphore"},
            {"for device"},
            {logical_device},
            {"signaling rendering is done"}}});

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    result = vkCreateFence(logical_device, &fence_info, nullptr, &in_flight_fence);

    claim({{{in_flight_fence},
            {"is a"},
            {"vulkan fence"},
            {"for device"},
            {logical_device},
            {"signaling drawing is complete"}}});
  }

  void init() override {
    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        TupleNoun::prefix()
    }});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan semaphore"))) {

      auto semaphore = static_cast<VkSemaphore>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());

      vkDestroySemaphore(logical_device, semaphore, nullptr);
    }

    if (t.matches(2, std::string("vulkan fence"))) {

      auto fence = static_cast<VkFence>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());

      vkDestroyFence(logical_device, fence, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSyncObjectsHandler);