#include <foxtalk_handler.hpp>

class VulkanConfigHandler : public Handler
{


protected:
  void handle(const std::vector<Tuple> &queryResults) override
  {
    bool should_include_validation_layers = true;
    uint64_t num_frames_in_flight = 2;
    claim({{{"vulkan"}, {"should be"}, {"running"}}});
    claim({{{"vulkan"}, {"should have"}, {num_frames_in_flight}, {"frames in flight"}}});
    if (should_include_validation_layers) {
      claim({{{"vulkan"}, {"should have"}, {"validation layers"}}});
    }
  
  }

  void init() override
  {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }


};

FOXTALK_FFI_HANDLER_REG(VulkanConfigHandler);