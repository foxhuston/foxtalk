#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "boot/VkDisplay.hpp"
#include "foxtalk.hpp"

#include "Reactor.h"
#include "DynamicCompilingHandler.h"

int main() {
  foxtalk::Reactor reactor{};
  foxtalk::DynamicCompilingHandler d{
      &reactor, "../src/foxtalk_handlers/",
      "-I/usr/include/opencv4 "
      "-I../c_reactor/lib "
      "-I../c_reactor/vendor/gc-8.2.4/include" // TODO: grumble.
  };

  Glfw container{};
  // VkDisplay container {};

  Core core(&container);

  const auto fbSize = container.getFramebufferSize();
  Foxtalk foxtalk{
      reactor,
      core.physicalDevice(),
      core.device(),
      core.renderPass(),
      static_cast<float>(fbSize.width),
      static_cast<float>(fbSize.height),
      MAX_FRAMES_IN_FLIGHT};

  container.mainLoop([&core, &foxtalk, &reactor]() {
    core.withRenderPass([&foxtalk, &reactor](const vk::CommandBuffer &cmdBuffer, auto renderPassBeginInfo, auto swapchainExtent,
                                   auto imageIndex) {
      reactor.tick();
      foxtalk.render(cmdBuffer, renderPassBeginInfo, swapchainExtent, imageIndex);
    });
    core.incrementFrame();
  });

  core.device().waitIdle();

  return 0;
}
