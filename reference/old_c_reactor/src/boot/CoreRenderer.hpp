//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_CORE_RENDERER_H
#define FOXTALK_CORE_RENDERER_H

#include <functional>
#include <vector>
#include <vulkan/vulkan.hpp>

class CoreRenderer {
public:
  CoreRenderer() { }
  virtual ~CoreRenderer() { }

  virtual std::vector<const char*> extensions() const = 0;
  virtual int rankPhysicalDevice(const vk::PhysicalDevice&) const = 0;
  virtual vk::SurfaceKHR createRenderSurface(const vk::Instance&, const vk::PhysicalDevice&) = 0;
  virtual vk::Extent2D getFramebufferSize() const = 0;

  virtual void setResizeCallback(std::function<void(int, int)>) = 0;
};

#endif // FOXTALK_CORE_RENDERER_H
