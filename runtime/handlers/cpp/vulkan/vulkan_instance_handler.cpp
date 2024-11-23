// write camera data to a texture
// assign texture to a quad (making a material)
// render a quad

// pkg-config: vulkan

#include <iostream>
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
    // std::cout << "available extensions:\n";



    std::vector<const char *> enabled_extensions {};

    for (const auto& extension : extensions) {
      for (const auto& desired_extension: optional_extensions) {
        if (desired_extension == extension.extensionName) {
          enabled_extensions.push_back(extension.extensionName);
        }
      }
    }

    for (auto i: enabled_extensions) {
      std::cout << i << std::endl;
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
      throw std::runtime_error("failed to create instance!");
    }

    std::cout << "Created Instance!!" << std::endl;

    // Put <CPtr(instance), "is the", "vulkan instance"> into db/*  */
    claim({{{instance}, {"is the"}, {"vulkan instance"}}});
  }

  void init() override
  {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanInstanceHandler);