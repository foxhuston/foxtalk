// pkg-config: vulkan

#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.hpp>

class VulkanLogicalDeviceHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() == 0) { return; }
    auto result = queryResults[0];
    auto chosen_queue_family = result.at<uint64_t>(4).value();
    auto queue_count = result.at<uint64_t>(6);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = chosen_queue_family;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    VkDevice logical_device;

    auto device =  static_cast<VkPhysicalDevice>(result.at<void *>(0).value());

    

    if (vkCreateDevice(device, &createInfo, nullptr, &logical_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(logical_device, chosen_queue_family, 0, &graphicsQueue);

     
    claim({{
      {logical_device}, {"is the"}, {"vulkan logical device"}, {"with graphics queue"}, {graphicsQueue}, {"with queue family index"}, { chosen_queue_family },
    }});
  }

  void init() override {
    
    claim({{
      TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}, {"with queue family index"}, TupleNoun::query(), TupleNoun::prefix()
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanLogicalDeviceHandler);