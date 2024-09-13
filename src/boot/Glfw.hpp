//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_GLFW_H
#define FOXTALK_TEST_GLFW_H

#include "CoreRenderer.hpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "CoreRenderer.hpp"



class Glfw : public CoreRenderer {
public:
  Glfw() {

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "Dust", nullptr, nullptr);
  }

  std::vector<const char*> extensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
  };

  int rankPhysicalDevice(const vk::PhysicalDevice&) const {
    return 0; // For now, just take the first physical device.
  }

  vk::SurfaceKHR createRenderSurface(const vk::Instance& instance) {
    VkSurfaceKHR surf;
    if(glfwCreateWindowSurface(instance, window, nullptr, &surf) != VK_SUCCESS) {
      throw std::runtime_error("GLFW failed to create window surface!");
    }

    return vk::SurfaceKHR(surf);
  }

  vk::Extent2D getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return vk::Extent2D(
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height));
  };

  void mainLoop(std::function<void(void)> appTick) {
    while(!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      appTick();
    }
    std::cout << "glfw should close!" << std::endl;
  }

  ~Glfw() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  private:
    GLFWwindow *window;
};


#endif //FOXTALK_TEST_GLFW_H
