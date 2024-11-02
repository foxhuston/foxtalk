// pkg-config: vulkan

#include <iostream>
#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>
#include <foxtalk_handler.hpp>

class ExampleHandler : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {
    if (queryResults.size() == 0)
    {
      return;
    }

    assert(queryResults.size() == 1);

    auto t = queryResults[0];

    VkInstance instance = static_cast<VkInstance>(t.at<void *>(0).value());
    std::cout << "Instance is: " << instance << std::endl;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
      claim({{{"vulkan"}, {"error"}, {"failed to find GPUs with Vulkan support!"}}});
    }
    else
    {
      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

      for (auto d : devices)
      {
        vk::PhysicalDevice dev(d);
        auto props = dev.getProperties();
        std::string device_name(props.deviceName);

        claim({{{d}, {"is a"}, {"vulkan physical device"},
              {"with name"}, {device_name},
              {"with type"}, {(uint64_t)props.deviceType}
              
        }});
      }
    }
  }

  void init() override
  {
    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(ExampleHandler);