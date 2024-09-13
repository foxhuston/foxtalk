#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "foxtalk.hpp"

int main() {
  Glfw container {}; // TODO: write VK_KHR_DISPLAY_EXTENSION version

  Core core(&container);
  Foxtalk foxtalk { };

  container.mainLoop([&core, &foxtalk]() {
    core.withRenderPass([&foxtalk](const vk::CommandBuffer &cmdBuffer) {
        foxtalk.render(cmdBuffer);
    });
  });

  core.device().waitIdle();

  return 0;
}
