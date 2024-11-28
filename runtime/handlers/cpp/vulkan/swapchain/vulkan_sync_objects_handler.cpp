// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanSyncObjectsHandler : public Handler {
public:
protected:
  std::vector<VkSemaphore> image_available_semaphores{};
  std::vector<VkSemaphore> render_finished_semaphores{};
  std::vector<VkFence> in_flight_fences{};

  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 3) {
      return;
    }

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

    for (uint64_t i = 0; i < frames_in_flight; i++) {
      VkSemaphoreCreateInfo semaphore_create_info{
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      };

      image_available_semaphores.emplace_back(VkSemaphore{});

      render_finished_semaphores.emplace_back(VkSemaphore{});

      in_flight_fences.emplace_back(VkFence{});

      auto result = vkCreateSemaphore(logical_device, &semaphore_create_info,
                                      nullptr, &image_available_semaphores[i]);
      result = vkCreateSemaphore(logical_device, &semaphore_create_info,
                                 nullptr, &render_finished_semaphores[i]);
      claim({{{image_available_semaphores[i]},
              {"is a"},
              {"vulkan semaphore"},
              {"for device"},
              {logical_device},
              {"signaling image is available"},
              {"for index"},
              {i}}});
      claim({{{render_finished_semaphores[i]},
              {"is a"},
              {"vulkan semaphore"},
              {"for device"},
              {logical_device},
              {"signaling rendering is done"},
              {"for index"},
              {i}}});

      VkFenceCreateInfo fence_info{};
      fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      result =
          vkCreateFence(logical_device, &fence_info, nullptr, &in_flight_fences[i]);

      claim({{{in_flight_fences[i]},
              {"is a"},
              {"vulkan fence"},
              {"for device"},
              {logical_device},
              {"signaling drawing is complete"},
              {"for index"},
              {i}}});
    }
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});

    claim({{{"vulkan"},
            {"should have"},
            TupleNoun::query(),
            {"frames in flight"}}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan semaphore"))) {

      auto semaphore = static_cast<VkSemaphore>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());
      debug << "Freeing semaphore " << t << end;
      // vkDestroySemaphore(logical_device, semaphore, nullptr);
    }

    if (t.matches(2, std::string("vulkan fence"))) {

      auto fence = static_cast<VkFence>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());
      debug << "Freeing fence " << t << end;
      // vkDestroyFence(logical_device, fence, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSyncObjectsHandler);