// pkg-config: vulkan

#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.hpp>

class VulkanLogicalDeviceHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() != 2) { return; }


    auto result_data = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (result_data == queryResults.end()) {
      log_error("Query results did not include the vulkan logical device");
      return;
    }
    const auto& result = *result_data;

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

    std::vector<const char*> required_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.ppEnabledExtensionNames = required_extensions.data();
    createInfo.enabledExtensionCount = required_extensions.size();
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
    
    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
    claim({{
      TupleNoun::query(), {"is the"}, {"chosen vulkan physical device"}, {"with queue family index"}, TupleNoun::query(), TupleNoun::prefix()
    }});
  }

  void free_tuple(const Tuple &t) override {
    
    if (t.matches(2, std::string("vulkan logical device"))) {
      
      auto logical_device = static_cast<VkDevice>(t.at<void *>(0).value());

      vkDestroyDevice(logical_device, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanLogicalDeviceHandler);