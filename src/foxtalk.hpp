//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

#include <array>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Vertex.hpp"

class Foxtalk {
  public:
    ///// CONSTRUCTOR //////////////////////////////////////////////////////////
    Foxtalk(
        // TODO: Needing both the device & physicacalDevice here feels like a
        //       leaky abstraction...
        const vk::PhysicalDevice *physicalDevice,
        const vk::Device *device,
        float framebufferWidth,
        float framebufferHeight
    )
      : _physicalDevice { physicalDevice }
      , _device { device }
      , _framebufferWidth { framebufferWidth }
      , _framebufferHeight { framebufferHeight }
    {
      updateProjectionMatrix();

      _vertices = {
          {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
      };

      ///// VERTEX BUFFER SETUP ////////////////////////////////////////////////

      vk::BufferCreateInfo vertexBufferInfo {
        {}
        , sizeof(_vertices[0]) * _vertices.size()
        , vk::BufferUsageFlagBits::eVertexBuffer
      };

      _vertexBuffer = device->createBuffer(vertexBufferInfo);

      ///// VERTEX BUFFER MEMORY ALLOCATION ////////////////////////////////////
      auto vertexBufferMemReq = _device->getBufferMemoryRequirements(_vertexBuffer);
      vk::MemoryAllocateInfo vertexAllocInfo {
        vertexBufferMemReq.size
        , findMemoryType(vertexBufferMemReq.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      _vertexBufferMemory = _device->allocateMemory(vertexAllocInfo);

      ///// VERTEX BUFFER MEMORY BINDING ///////////////////////////////////////
      _device->bindBufferMemory(_vertexBuffer, _vertexBufferMemory, 0);
      
      ///// VERTEX BUFFER MEMORY FILLING ///////////////////////////////////////
      void* data = _device->mapMemory(_vertexBufferMemory, 0, vertexBufferInfo.size);
        memcpy(data, _vertices.data(), (size_t) vertexBufferInfo.size);
      _device->unmapMemory(_vertexBufferMemory);
    }

    // TODO: This shouldn't be here (see upper TODO...)
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
      auto memProperties = _physicalDevice->getMemoryProperties();
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
          if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
              return i;
          }
      }

      throw std::runtime_error("failed to find suitable memory type!");
    }

    ///// DESTRUCTOR ///////////////////////////////////////////////////////////
    ~Foxtalk() {
      _device->destroyBuffer(_vertexBuffer);
      _device->freeMemory(_vertexBufferMemory);
    }

    ///// DRAWING //////////////////////////////////////////////////////////////
    void tick() { }

    void render(const vk::CommandBuffer& cmdBuffer) {
      std::vector<vk::Buffer> buffs { _vertexBuffer };
      std::vector<vk::DeviceSize> offsets { 0 };

      cmdBuffer.bindVertexBuffers(0, buffs, offsets);
      cmdBuffer.draw(static_cast<uint32_t>(_vertices.size()), 1, 0, 0);
    }

    ///// GETTERS & SETTERS ////////////////////////////////////////////////////
    void setFramebufferWidth(float newWidth) {
      _framebufferWidth = newWidth;
      updateProjectionMatrix();
    }

    void setFramebufferHeight(float newHeight) {
      _framebufferHeight = newHeight;
      updateProjectionMatrix();
    }

  private:
    const vk::Device *_device;
    const vk::PhysicalDevice *_physicalDevice;

    float _framebufferWidth;
    float _framebufferHeight;

    float _far = 1.0;
    float _near = -1.0;

    glm::mat4 _projection;
    std::vector<Vertex> _vertices;

    vk::Buffer _vertexBuffer;
    vk::DeviceMemory _vertexBufferMemory;



    // Orthorgraphic.
    void updateProjectionMatrix() {
      _projection = {
        { 2 / _framebufferWidth , 0.0f                   , 0.0f               , -1.0f },
        { 0.0f                  , 2 / _framebufferHeight , 0.0f               , -1.0f },
        { 0.0f                  , 0.0f                   , 2 / (_far - _near) ,  0.0f },
        { 0.0f                  , 0.0f                   , 0.0f               ,  1.0f }
      };
    }

};

#endif // FOXTALK_FOXTALK_H
