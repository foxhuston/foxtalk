//foxtalk-link vulkan
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.hpp>

class VulkanPhysicalDeviceSelector : public Handler
{

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if(queryResults.size() == 0) { return; }

    if(queryResults.size() == 1) {
      auto result = queryResults[0];

      claim({{
        {result.at<void*>(0).value()}, {"is the"}, {"chosen vulkan physical device"}
      }});
      return;
    }
    
    log_error("Selecting which physical vulkan device out of a list is not implemented.");
    // for(auto& result : queryResults) {
    //   auto props = result.struct_at<vk::PhysicalDeviceProperties>(4).value();
    //   auto features = result.struct_at<vk::PhysicalDeviceFeatures>(6).value();


    //   log_debug("Found props with name " << props.deviceName);
    //   log_debug(std::boolalpha << "can device do geometry shaders? " << (bool)features.geometryShader);
    //   if (features.geometryShader) {

    //   }
    // }
  }


  void init() override {
    claim({{
      TupleNoun::query(), {"is a"}, {"vulkan physical device"},
              {"with props"}, TupleNoun::query(),
              {"with features"}, TupleNoun::query()
    }});
  }

};

FOXTALK_FFI_HANDLER_REG(VulkanPhysicalDeviceSelector);