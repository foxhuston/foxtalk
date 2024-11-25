// pkg-config: vulkan
#include <foxtalk_handler.hpp>

#include <vulkan/vulkan.hpp>

class VulkanQueueFamiliesHandler : public Handler {

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() == 0) {
      return;
    }
    auto result = queryResults[0];
    auto device = static_cast<VkPhysicalDevice>(result.at<void *>(0).value());

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    // TODO: be smarter about selecting a queue
    std::optional<uint32_t> chosen_queue_family{};
    uint64_t queue_count = 0;
    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        chosen_queue_family = i;
        queue_count = queueFamily.queueCount;
        break;
      }
      i++;
    }

    if (!chosen_queue_family.has_value()) {
      log_error("No queue families found with VK_QUEUE_GRAPHICS_BIT set");
    } else {

      claim({{{device},
              {"is the"},
              {"chosen vulkan physical device"},
              {"with queue family index"},
              {static_cast<uint64_t>(chosen_queue_family.value())},
              {"with queue count"},
              {queue_count}}});
    }
  }

  void init() override {

    claim(
        {{TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanQueueFamiliesHandler);