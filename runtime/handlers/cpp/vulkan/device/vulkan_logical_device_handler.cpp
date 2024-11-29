// pkg-config: vulkan

#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  [[nodiscard]] bool is_complete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class VulkanLogicalDeviceHandler : public Handler {

protected:
  VkDevice logical_device{};
  QueueFamilyIndices indices{};
  VkQueue graphicsQueue{};
  VkQueue presentQueue{};
  void handle(const std::vector<Tuple> &queryResults) override {
    auto surface_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vk surface khr";
        });

    if (surface_tuple == queryResults.end()) {
      err << "Query results did not include the surface tuple" << end;
      return;
    }

    auto surface =
        static_cast<VkSurfaceKHR>(surface_tuple->at<void *>(0).value());

    auto result_data = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "chosen vulkan physical device";
        });

    if (result_data == queryResults.end()) {
      err << "Query results did not include the vulkan device" << end;
      return;
    }
    const auto &result = *result_data;

    // auto chosen_queue_family = result.at<uint64_t>(4).value();
    // auto queue_count = result.at<uint64_t>(6);
    auto dev = static_cast<VkPhysicalDevice>(result.at<void *>(0).value());

    auto queue_families = findQueueFamilies(dev, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {
        queue_families.graphicsFamily.value(),
        queue_families.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();

    std::vector<const char *> required_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.ppEnabledExtensionNames = required_extensions.data();
    createInfo.enabledExtensionCount = required_extensions.size();

    auto validation_layer_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "validation layers";
        });

    if (surface_tuple != queryResults.end()) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
      // required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    } else {
      createInfo.enabledLayerCount = 0;
    }

    auto device = static_cast<VkPhysicalDevice>(result.at<void *>(0).value());

    
    if (
      logical_device != VK_NULL_HANDLE && 
      graphicsQueue != VK_NULL_HANDLE && 
      presentQueue != VK_NULL_HANDLE && 
      indices.graphicsFamily == queue_families.graphicsFamily &&
      indices.presentFamily == queue_families.presentFamily ) {
      debug << "Already have a logical device, re-asserting logical device!" << end;
      claim_device();
      return;
    }
    debug << "Creating new logical device! " << end;
    indices = queue_families;
    auto r = vkCreateDevice(device, &createInfo, nullptr, &logical_device);
    if (r != VK_SUCCESS) {
      err << "failed to create logical device: " << r << end;
      return;
    }
 
    vkGetDeviceQueue(logical_device, queue_families.graphicsFamily.value(), 0,
                     &graphicsQueue);

    if (queue_families.graphicsFamily == queue_families.presentFamily) {
      presentQueue = graphicsQueue;
    } else {
      vkGetDeviceQueue(logical_device, queue_families.presentFamily.value(), 0,
                       &presentQueue);
    }

    claim({{
        {logical_device},
        {"is the"},
        {"vulkan logical device"},
        {"with graphics queue"},
        {graphicsQueue},
        {"with family index"},
        {(uint64_t)queue_families.graphicsFamily.value()},
        {"with present queue"},
        {presentQueue},
        {"with family index"},
        {(uint64_t)queue_families.presentFamily.value()},
    }});
  }

  void claim_device() {
    
    claim({{
        {logical_device},
        {"is the"},
        {"vulkan logical device"},
        {"with graphics queue"},
        {graphicsQueue},
        {"with family index"},
        {(uint64_t)indices.graphicsFamily.value()},
        {"with present queue"},
        {presentQueue},
        {"with family index"},
        {(uint64_t)indices.presentFamily.value()},
    }});
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is a"},
            {"vk surface khr"},
            TupleNoun::prefix()}});
    claim({{{"vulkan"}, {"should have"}, {"validation layers"}}});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
    claim({{TupleNoun::query(),
            {"is the"},
            {"chosen vulkan physical device"},
            TupleNoun::prefix()}});
  }

  void free_tuple(const Tuple &t) override {

    if (t.matches(2, std::string("vulkan logical device"))) {

      auto logical_device = static_cast<VkDevice>(t.at<void *>(0).value());
      debug << "Freeing logical device " << t << end;
      vkDestroyDevice(logical_device, nullptr);
    }
  }

  // Taken directly from vulkan tutorial
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      // std::cout << queueFamily.queueCount << " eee " <<
      // queueFamily.queueFlags << std::endl;
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (indices.is_complete()) {
        break;
      }

      i++;
    }

    return indices;
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanLogicalDeviceHandler);