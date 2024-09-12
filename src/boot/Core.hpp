//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_CORE_H
#define FOXTALK_TEST_CORE_H

#include <iostream>
#include <set>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "CoreRenderer.hpp"
#include "QueueFamilies.hpp"
#include "SwapchainSupportDetails.hpp"

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
    Core(const Core &) = delete;
    Core(Core &&) = delete;
    Core &operator=(const Core &) = delete;
    Core &operator=(Core &&) = delete;

    Core(CoreRenderer *cr) : _coreRenderer{cr} {
      ///// BOOT LOGGING /////////////////////////////////////////////////////////
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

      ///// INITIAL SETUP ////////////////////////////////////////////////////////
      vk::ApplicationInfo appInfo(
          "Dust",
          vk::makeApiVersion(0, 0, 1, 0),
          "Dust Engine",
          vk::makeApiVersion(0, 0, 1, 0),
          VK_API_VERSION_1_3
      );

      std::vector<const char*> extensions = coreRenderer().extensions();
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

      ///// CREATE SURFACE ///////////////////////////////////////////////////////
      _outputSurface = coreRenderer().createRenderSurface(instance());

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
#endif

      ///// PHYSICAL DEVICE //////////////////////////////////////////////////////
      std::cout << "Found Physical Devices:" << std::endl;
      auto physicalDevices = instance().enumeratePhysicalDevices();

      // TODO: Check physical device for swapchain support...
      _physicalDevice = physicalDevices[0];
      int max_score = 0;
      for(const auto& pd : physicalDevices) {
        auto score = coreRenderer().rankPhysicalDevice(pd);
        std::cout << "\t" << pd.getProperties().deviceName << " (score " << score << ")" << std::endl;
        if(score > max_score) {
          _physicalDevice = pd;
          max_score = score;
        }
      }

      std::cout << "Selected " << _physicalDevice.getProperties().deviceName << std::endl;

      ///// QUEUE FAMILIES ///////////////////////////////////////////////////////
      auto queueFamilies = physicalDevice().getQueueFamilyProperties();
      int i = 0;
      for(const auto& qf : queueFamilies) {
        if(qf.queueFlags & vk::QueueFlagBits::eGraphics) {
          _queueFamilyIndices.graphicsFamily = i;
        }

        if(physicalDevice().getSurfaceSupportKHR(i, outputSurface())) {
          _queueFamilyIndices.presentFamily = i;
        }

        if(_queueFamilyIndices.isComplete()) {
          break;
        }

        i++;
      }

      assert(_queueFamilyIndices.isComplete());

      ///// LOGICAL DEVICE ///////////////////////////////////////////////////////
      std::vector<float> queuePriorities { 1.0f };
      std::vector<uint32_t> queueFamilyIndices = _queueFamilyIndices.indices();
      std::set<uint32_t> uniqueQueueFamilies { queueFamilyIndices.begin(), queueFamilyIndices.end() };

      std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
      for(auto queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(
          vk::DeviceQueueCreateInfo(
              {},
              _queueFamilyIndices.graphicsFamily.value(),
              queuePriorities
              ));
      }

      std::vector<const char*> enabledExtensions {
        vk::KHRSwapchainExtensionName
      };
          
      vk::PhysicalDeviceFeatures physicalDeviceFeatures;
      vk::DeviceCreateInfo deviceCreateInfo(
          {}
          , queueCreateInfos
          , validationLayers
          , enabledExtensions
          , &physicalDeviceFeatures
          );

      _device = physicalDevice().createDevice(deviceCreateInfo);

      ///// LOGICAL DEVICE QUEUES ////////////////////////////////////////////////
      _graphicsQueue = device().getQueue(_queueFamilyIndices.graphicsFamily.value(), 0);
      _presentQueue  = device().getQueue(_queueFamilyIndices.presentFamily.value(), 0);

      ///// SWAPCHAIN ////////////////////////////////////////////////////////////
      SwapchainSupportDetails swapChainSupportDetails(physicalDevice(), outputSurface());
      auto surfaceFormat = swapChainSupportDetails.chooseSwapSurfaceFormat();
      auto presentMode   = swapChainSupportDetails.chooseSwapPresentMode();
      _swapchainExtent   = swapChainSupportDetails.chooseSwapExtent(coreRenderer().getFramebufferSize());

      uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
      if(swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount) {
        imageCount = swapChainSupportDetails.capabilities.maxImageCount;
      }

      vk::SwapchainCreateInfoKHR swapchainCreateInfo(
          {}, outputSurface(), imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
          _swapchainExtent, 1, vk::ImageUsageFlagBits::eColorAttachment);

      if(_queueFamilyIndices.graphicsFamily != _queueFamilyIndices.presentFamily) {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchainCreateInfo.setQueueFamilyIndices(queueFamilyIndices);
      } else {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
      }
      
      swapchainCreateInfo.setPreTransform(swapChainSupportDetails.capabilities.currentTransform);
      swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

      _swapchain = device().createSwapchainKHR(swapchainCreateInfo);

      ///// SWAPCHAIN IMAGES /////////////////////////////////////////////////////
      _swapchainImages = device().getSwapchainImagesKHR(swapchain());
      _swapchainImageFormat = surfaceFormat.format;

      ///// SWAPCHAIN IMAGE VIEWS ////////////////////////////////////////////////
      for(auto& image : swapchainImages()) {
        vk::ImageViewCreateInfo imageViewCreateInfo(
            {}, image, vk::ImageViewType::e2D, swapchainImageFormat());

        imageViewCreateInfo.components.setR(vk::ComponentSwizzle::eIdentity);
        imageViewCreateInfo.components.setG(vk::ComponentSwizzle::eIdentity);
        imageViewCreateInfo.components.setB(vk::ComponentSwizzle::eIdentity);
        imageViewCreateInfo.components.setA(vk::ComponentSwizzle::eIdentity);

        imageViewCreateInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        imageViewCreateInfo.subresourceRange.setBaseMipLevel(0);
        imageViewCreateInfo.subresourceRange.setLevelCount(1);
        imageViewCreateInfo.subresourceRange.setBaseArrayLayer(0);
        imageViewCreateInfo.subresourceRange.setLayerCount(1);

        _swapchainImageViews.push_back(device().createImageView(imageViewCreateInfo));
      }
    };

    ///// DESTRUCTOR /////////////////////////////////////////////////////////////
    ~Core() {
      for(auto imageView : _swapchainImageViews) {
        _device.destroyImageView(imageView);
      }

      _device.destroySwapchainKHR(_swapchain);
      _device.destroy();
      
#ifndef NDEBUG
      _instance.destroyDebugUtilsMessengerEXT(_debugUtilsMessenger);
#endif
      _instance.destroySurfaceKHR(_outputSurface);
      _instance.destroy();
    };

    ///// GETTERS & SETTERS //////////////////////////////////////////////////////
    const vk::Instance& instance() const {
      return _instance;
    }

    const vk::PhysicalDevice& physicalDevice() const {
      return _physicalDevice;
    }

    const vk::Device& device() const {
      return _device;
    }

    const vk::Queue graphicsQueue() const {
      return _graphicsQueue;
    }

    const vk::Queue presentQueue() const {
      return _presentQueue;
    }

    const vk::SurfaceKHR outputSurface() const {
      return _outputSurface;
    }

    CoreRenderer& coreRenderer() const {
      return *_coreRenderer;
    }

    const vk::SwapchainKHR swapchain() const {
      return _swapchain;
    }

    const std::vector<vk::Image> swapchainImages() const {
      return _swapchainImages;
    }

    const vk::Extent2D swapchainExtent() const {
      return _swapchainExtent;
    }

    const vk::Format swapchainImageFormat() const {
      return _swapchainImageFormat;
    }

    const std::vector<vk::ImageView> swapchainImageViews() const {
      return _swapchainImageViews;
    }

private:
    CoreRenderer *_coreRenderer;

    vk::Instance _instance;
    vk::PhysicalDevice _physicalDevice;
    vk::Device _device;
    vk::Queue _graphicsQueue;
    vk::Queue _presentQueue;
    vk::SurfaceKHR _outputSurface;
    vk::SwapchainKHR _swapchain;
    std::vector<vk::Image> _swapchainImages;
    vk::Extent2D _swapchainExtent;
    vk::Format _swapchainImageFormat;
    std::vector<vk::ImageView> _swapchainImageViews;

    QueueFamiliyIndices _queueFamilyIndices;

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
