//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_GLFW_H
#define FOXTALK_TEST_GLFW_H

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Core.h"



class Glfw {
public:
  Glfw() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "Dust", nullptr, nullptr);

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    _core = new Core(
        requiredExtensions,
        // Take the first physical device.
        [](const vk::PhysicalDevice& pd) { return 0; },
        [&](const vk::Instance& instance) {
          VkSurfaceKHR surf;
          if(glfwCreateWindowSurface(instance, window, nullptr, &surf) != VK_SUCCESS) {
            throw std::runtime_error("GLFW failed to create window surface!");
          }

          return vk::SurfaceKHR(surf);
        }
      );
  }

  Core& core() const {
    return *_core;
  }

  void mainLoop() {
    while(!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
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
