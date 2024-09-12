//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_GLFW_H
#define FOXTALK_TEST_GLFW_H

#include <iostream>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Core.h"



class Glfw {
public:
  Glfw() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    _core = new Core(
        requiredExtensions,
        // Take the first physical device.
        [](vk::PhysicalDeviceProperties props) { return 0; }
      );
  }

  Core& core() const {
    return *_core;
  }

  ~Glfw() {
    delete _core;
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  private:
    Core *_core;
    GLFWwindow *window;
};


#endif //FOXTALK_TEST_GLFW_H
