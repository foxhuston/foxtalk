#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "foxtalk.hpp"

int main() {
  Glfw container {}; // TODO: write VK_KHR_DISPLAY_EXTENSION version

  Core core(&container);

  const auto fbSize = container.getFramebufferSize();
  Foxtalk foxtalk (fbSize.width, fbSize.height);

  container.mainLoop([&core, &foxtalk]() {
    core.withRenderPass([&foxtalk](const vk::CommandBuffer &cmdBuffer) {
        foxtalk.render(cmdBuffer);
    });
    core.incrementFrame();
  });

  core.device().waitIdle();

  return 0;
}
