//foxtalk-link vulkan

#include <iostream>
#include <vulkan/vulkan.h>
#include <foxtalk_handler.hpp>

class ExampleHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {

    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

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

FOXTALK_FFI_HANDLER_REG(ExampleHandler);