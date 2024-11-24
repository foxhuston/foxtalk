// write camera data to a texture
// assign texture to a quad (making a material)
// render a quad

// pkg-config: vulkan

#include <string_view>

#include <vulkan/vulkan.h>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>


class VulkanInstanceHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {

    std::vector<std::string_view> optional_extensions {
      VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
      VK_KHR_DISPLAY_EXTENSION_NAME,
      VK_KHR_SURFACE_EXTENSION_NAME
    };



    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Foxtalk";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    // log_debug("available extensions:\n";



    std::vector<const char *> enabled_extensions {};

    for (const auto& extension : extensions) {
      for (const auto& desired_extension: optional_extensions) {
        if (desired_extension == extension.extensionName) {
          enabled_extensions.push_back(extension.extensionName);
        }
      }
    }

    for (auto i: enabled_extensions) {
      log_debug(i);
    } 

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = enabled_extensions.size();
    createInfo.ppEnabledExtensionNames = enabled_extensions.data();
    
    std::vector<const char*> required_layers{"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = required_layers.size();
    createInfo.ppEnabledLayerNames = required_layers.data();
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
      log_error("Failed to create vk instance..");
      return;
    }

    log_debug("Created Instance!!");

    // Put <CPtr(instance), "is the", "vulkan instance"> into db/*  */
    claim({{{instance}, {"is the"}, {"vulkan instance"}}});
  }

  void init() override
  {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan instance"))) {
      auto instance = static_cast<VkInstance>(t.at<void *>(0).value());
      std::cerr << "We should have killed the vulkan instance... but because of the way deletes "
        << "in vulkan work, we can't actually delete it until everything downstream of it has "
        << "already been deleted. In Foxtalk, this DOES happen due to the reactive nature  " 
        << "of the reactor, but there's an ordering issue here at play. We should only call free tuple "
        << "once everything has gone through a tick of it being gone. " << std::endl;
      // vkDestroyInstance(instance, nullptr);
    }
  }

};

FOXTALK_FFI_HANDLER_REG(VulkanInstanceHandler);