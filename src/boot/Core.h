//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_CORE_H
#define FOXTALK_TEST_CORE_H

#include <functional>
#include <iostream>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>


#ifndef NDEBUG
static PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

// N.B. we have to manually define these functions here so that they're picked up
// by the C++ API. This seems pretty brittle, but it's how the actual examples
// seem to do it; see: https://github.com/KhronosGroup/Vulkan-Hpp/blob/4e6e8d3fda12aa70fd1d3c08cbeee092d5ebb19a/samples/CreateDebugUtilsMessenger/CreateDebugUtilsMessenger.cpp
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT( VkInstance                                 instance,
                                                               const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
                                                               const VkAllocationCallbacks *              pAllocator,
                                                               VkDebugUtilsMessengerEXT *                 pMessenger )
{
  return pfnVkCreateDebugUtilsMessengerEXT( instance, pCreateInfo, pAllocator, pMessenger );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const * pAllocator )
{
  return pfnVkDestroyDebugUtilsMessengerEXT( instance, messenger, pAllocator );
}

#endif


class Core {
  public:
    Core(
        std::vector<const char*> extensions,
        std::function<int(vk::PhysicalDeviceProperties)> rankPhysicalDevice
    ) {
      ///// INFO LOGGING /////
      auto availableExtensions = vk::enumerateInstanceExtensionProperties();
      std::cout << "Available Extensions:" << std::endl;
      for(const auto& extension : availableExtensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
      }

      auto availableLayers = vk::enumerateInstanceLayerProperties();
      std::cout << "Available Layers:" << std::endl;
      for(const auto& layer : availableLayers) {
        std::cout << "\t" << layer.layerName << std::endl;
      }

      ///// SETUP ////////////
      vk::ApplicationInfo appInfo(
          "Dust",
          vk::makeApiVersion(0, 0, 1, 0),
          "Dust Engine",
          vk::makeApiVersion(0, 0, 1, 0),
          VK_API_VERSION_1_3
      );

      std::vector<const char*> validationLayers {};

#ifndef NDEBUG
      validationLayers.push_back("VK_LAYER_KHRONOS_validation");
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

      std::cout << "Creating with Required Extensions:" << std::endl;
      for(const auto& extensionName : extensions) {
        std::cout << "\t" << extensionName << std::endl;
      }

      vk::InstanceCreateInfo instanceInfo(
          {}, &appInfo, validationLayers, extensions
      );

      _instance = vk::createInstance(instanceInfo);

      ///// INSTANCE CREATED! ////////////////////////////////////////////////////

      ///// VALIDATION LAYERS ////////////////////////////////////////////////////

#ifndef NDEBUG
      pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( instance().getProcAddr( "vkCreateDebugUtilsMessengerEXT" ) );
      if ( !pfnVkCreateDebugUtilsMessengerEXT )
      {
        std::cout << "GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function." << std::endl;
        exit( 1 );
      }

      pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( instance().getProcAddr( "vkDestroyDebugUtilsMessengerEXT" ) );
      if ( !pfnVkDestroyDebugUtilsMessengerEXT )
      {
        std::cout << "GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function." << std::endl;
        exit( 1 );
      }

      auto debugUtilsCreationInfo = vk::DebugUtilsMessengerCreateInfoEXT(
          {},
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            /* | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo */
            /* | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose */
          ,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
          ,
          Core::debugCallback
      );

      _debugUtilsMessenger = _instance.createDebugUtilsMessengerEXT(debugUtilsCreationInfo);

      ///// PHYSICAL DEVICE //////////////////////////////////////////////////////
      std::cout << "Found Physical Devices:" << std::endl;
      auto physicalDevices = instance().enumeratePhysicalDevices();

      _physicalDevice = physicalDevices[0];
      int max_score = 0;
      for(const auto& pd : physicalDevices) {
        auto props = pd.getProperties();
        auto score = rankPhysicalDevice(props);
        std::cout << "\t" << props.deviceName << " (score " << score << ")" << std::endl;
        if(score > max_score) {
          _physicalDevice = pd;
          max_score = score;
        }
      }

      std::cout << "Selected " << _physicalDevice.getProperties().deviceName << std::endl;

    };
#endif

    ///// DESTRUCTOR /////////////////////////////////////////////////////////////
    ~Core() {
#ifndef NDEBUG
      _instance.destroyDebugUtilsMessengerEXT(_debugUtilsMessenger);
#endif
      _instance.destroy();
    };

    ///// GETTERS & SETTERS //////////////////////////////////////////////////////
    const vk::Instance instance() {
      return _instance;
    }

    const vk::PhysicalDevice physicalDevice() {
      return _physicalDevice;
    }

private:
    vk::Instance _instance;
    vk::PhysicalDevice _physicalDevice;

#ifndef NDEBUG
    vk::DebugUtilsMessengerEXT _debugUtilsMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

      switch(messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        std::cerr << "\x1b[38:5:7m [Verbose]: ";
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        std::cerr << "\x1b[38:5:85m [Info]: ";
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        std::cerr << "\x1b[38:5:226m [Warning]: ";
        break;
      default:
        std::cerr << "\x1b[38:5:197m [Error]: ";
        break;
      }

      std::cerr << pCallbackData->pMessage << "\x1b[m" << std::endl;

      return VK_FALSE;
    }
#endif
};


#endif //FOXTALK_TEST_CORE_H
