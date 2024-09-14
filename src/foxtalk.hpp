//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

#include "boot/shader.hpp"
#include <array>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "boot/GraphicsPipeline.hpp"
#include "Vertex.hpp"

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class Foxtalk {
  public:
    ///// CONSTRUCTOR //////////////////////////////////////////////////////////
    Foxtalk(
        // TODO: Needing both the device & physicacalDevice here feels like a
        //       leaky abstraction...
        const vk::PhysicalDevice &physicalDevice,
        const vk::Device &device,
        const vk::RenderPass &renderPass,
        float framebufferWidth,
        float framebufferHeight
    )
      : _physicalDevice { physicalDevice }
      , _device { device }
      , _framebufferWidth { framebufferWidth }
      , _framebufferHeight { framebufferHeight }
    {
      updateProjectionMatrix();

      std::cout << "device? " << device << std::endl;

      ///// PIPELINE SETUP /////////////////////////////////////////////////////
      Shader vertexShader(&device, "src/shaders/simple.vert.bin");
      Shader fragmentShader(&device, "src/shaders/simple.frag.bin");
      _pipeline = GraphicsPipeline<Vertex>(
        device,
        renderPass,
        std::move(vertexShader),
        std::move(fragmentShader)
      );

      ///// TEMP DATA //////////////////////////////////////////////////////////

      _vertices = {
          {{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
          {{0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
          {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
          {{1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
      };

      _indices = {
        0, 1, 2, 2, 3, 0
      };

      ///// VERTEX BUFFER SETUP ////////////////////////////////////////////////

      vk::BufferCreateInfo vertexBufferInfo {
        {}
        , sizeof(_vertices[0]) * _vertices.size()
        , vk::BufferUsageFlagBits::eVertexBuffer
      };

      _vertexBuffer = _device.createBuffer(vertexBufferInfo);

      ///// VERTEX BUFFER MEMORY ALLOCATION ////////////////////////////////////
      auto vertexBufferMemReq = _device.getBufferMemoryRequirements(_vertexBuffer);
      vk::MemoryAllocateInfo vertexAllocInfo {
        vertexBufferMemReq.size
        , findMemoryType(vertexBufferMemReq.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      _vertexBufferMemory = _device.allocateMemory(vertexAllocInfo);

      ///// VERTEX BUFFER MEMORY BINDING ///////////////////////////////////////
      _device.bindBufferMemory(_vertexBuffer, _vertexBufferMemory, 0);
      
      ///// VERTEX BUFFER MEMORY FILLING ///////////////////////////////////////
      void* vertexData = _device.mapMemory(_vertexBufferMemory, 0, vertexBufferInfo.size);
        memcpy(vertexData, _vertices.data(), (size_t) vertexBufferInfo.size);
      _device.unmapMemory(_vertexBufferMemory);

      ///// VERTEX BUFFER SETUP ////////////////////////////////////////////////

      vk::BufferCreateInfo indexBufferInfo {
        {}
        , sizeof(_indices[0]) * _indices.size()
        , vk::BufferUsageFlagBits::eIndexBuffer
      };

      _indexBuffer = _device.createBuffer(indexBufferInfo);

      ///// VERTEX BUFFER MEMORY ALLOCATION ////////////////////////////////////
      auto indexBufferMemReq = _device.getBufferMemoryRequirements(_indexBuffer);
      vk::MemoryAllocateInfo indexAllocInfo {
        indexBufferMemReq.size
        , findMemoryType(indexBufferMemReq.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      _indexBufferMemory = _device.allocateMemory(indexAllocInfo);

      ///// VERTEX BUFFER MEMORY BINDING ///////////////////////////////////////
      _device.bindBufferMemory(_indexBuffer, _indexBufferMemory, 0);
      
      ///// VERTEX BUFFER MEMORY FILLING ///////////////////////////////////////
      void* indexData = _device.mapMemory(_indexBufferMemory, 0, indexBufferInfo.size);
        memcpy(indexData, _indices.data(), (size_t) indexBufferInfo.size);
      _device.unmapMemory(_indexBufferMemory);

      ///// UNIFORM BUFFER DESCRIPTOR SET //////////////////////////////////////
      vk::DescriptorSetLayoutBinding uboLayoutBinding {
        0, vk::DescriptorType::eUniformBuffer, 1
        , vk::ShaderStageFlagBits::eVertex
      };



    }

    // TODO: This shouldn't be here (see upper TODO...)
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
      auto memProperties = _physicalDevice.getMemoryProperties();
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
          if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
              return i;
          }
      }

      throw std::runtime_error("failed to find suitable memory type!");
    }

    ///// DESTRUCTOR ///////////////////////////////////////////////////////////
    ~Foxtalk() {
      _device.destroyBuffer(_vertexBuffer);
      _device.freeMemory(_vertexBufferMemory);

      _device.destroyBuffer(_indexBuffer);
      _device.freeMemory(_indexBufferMemory);
    }

    ///// DRAWING //////////////////////////////////////////////////////////////
    void tick() { }

    void render(const vk::CommandBuffer& commandBuffer, const vk::Extent2D swapchainExtent) {
      std::vector<vk::Buffer> buffs { _vertexBuffer };
      std::vector<vk::DeviceSize> offsets { 0 };

      // TODO: Maybe a vulkan pipeline == a material??
      //       I think maybe everything from here to endRenderPass might should be 
      //       in the actual drawables...
      commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.pipeline());

      commandBuffer.setViewport(0, {{
          0.0f
          , 0.0f
          , static_cast<float>(swapchainExtent.width)
          , static_cast<float>(swapchainExtent.height)
          , 0.0f
          , 1.0f
        }});

      std::vector<vk::Rect2D> scissors {
        {
          {0, 0}
          , swapchainExtent
        }
      };

      commandBuffer.setScissor(0, scissors);

      commandBuffer.bindVertexBuffers(0, buffs, offsets);
      commandBuffer.bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint32);;
      commandBuffer.drawIndexed(static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
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
    const vk::Device& _device;
    const vk::PhysicalDevice& _physicalDevice;

    GraphicsPipeline<Vertex> _pipeline;

    float _framebufferWidth;
    float _framebufferHeight;

    float _far = 1.0;
    float _near = -1.0;

    glm::mat4 _projection;

    std::vector<Vertex> _vertices;
    vk::Buffer _vertexBuffer;
    vk::DeviceMemory _vertexBufferMemory;

    std::vector<uint32_t> _indices;
    vk::Buffer _indexBuffer;
    vk::DeviceMemory _indexBufferMemory;


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
