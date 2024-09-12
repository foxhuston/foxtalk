#include "boot/Glfw.h"

int main() {
  Glfw container {}; // TODO: write VK_KHR_DISPLAY_EXTENSION version

  container.mainLoop();

  return 0;
}
