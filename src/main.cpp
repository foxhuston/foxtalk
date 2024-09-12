#include "boot/Core.hpp"
#include "boot/Glfw.hpp"
#include "foxtalk.hpp"

int main() {
  Glfw container {}; // TODO: write VK_KHR_DISPLAY_EXTENSION version

  Core core(&container);

  container.mainLoop();

  return 0;
}
