#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "foxtalk.hpp"

int main() {
  Glfw container {}; // TODO: write VK_KHR_DISPLAY_EXTENSION version

  Core core(&container);

  const auto fbSize = container.getFramebufferSize();
  Foxtalk foxtalk {
    &core.physicalDevice(),
    &core.device(),
    static_cast<float>(fbSize.width),
    static_cast<float>(fbSize.height) };

  container.mainLoop([&core, &foxtalk]() {
    core.withRenderPass([&foxtalk](const vk::CommandBuffer &cmdBuffer) {
        foxtalk.render(cmdBuffer);
    });
    core.incrementFrame();
  });

  core.device().waitIdle();

  return 0;
}
