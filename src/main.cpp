#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "boot/VkDisplay.hpp"
#include "foxtalk.hpp"

int main() {
  Glfw container{};
  // VkDisplay container {};

  Core core(&container);

  const auto fbSize = container.getFramebufferSize();
  Foxtalk foxtalk{
      core.physicalDevice(),
      core.device(),
      core.renderPass(),
      static_cast<float>(fbSize.width),
      static_cast<float>(fbSize.height),
      MAX_FRAMES_IN_FLIGHT};

  container.mainLoop([&core, &foxtalk]() {
    core.withRenderPass([&foxtalk](const vk::CommandBuffer &cmdBuffer, auto renderPassBeginInfo, auto swapchainExtent,
                                   auto imageIndex) {
      foxtalk.render(cmdBuffer, renderPassBeginInfo, swapchainExtent, imageIndex);
    });
    core.incrementFrame();
  });

  core.device().waitIdle();

  return 0;
}
