// pkg-config: vulkan

#include <iostream>
#include <vector>

#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.hpp>

class VulkanPhysicalDeviceHandler : public Handler {

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() == 0) {
      return;
    }

    assert(queryResults.size() == 1);

    auto t = queryResults[0];

    VkInstance instance = static_cast<VkInstance>(t.at<void *>(0).value());

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      claim({{{"vulkan"},
              {"error"},
              {"failed to find GPUs with Vulkan support!"}}});
    } else {
      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

      debug << "found " << deviceCount << " devices" << this;

      for (auto d : devices) {
        vk::PhysicalDevice dev(d);
        auto props = dev.getProperties();
        auto features = dev.getFeatures();

        claim({{{d},
                {"is a"},
                {"vulkan physical device"},
                {"with props"},
                TupleNoun::from_struct(props),
                {"with features"},
                TupleNoun::from_struct(features),
        }});
      }
    }
  }

  void init() override {
    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanPhysicalDeviceHandler);