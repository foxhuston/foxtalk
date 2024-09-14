//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

#include "boot/shader.hpp"
#include <array>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "boot/GraphicsPipeline.hpp"
#include "Vertex.hpp"

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

template<typename T>
struct VBuffer {
  ///// NO COPY ONLY MOVE BARK BARK //////////////////////////////////////////
  VBuffer() { }

  VBuffer(const VBuffer &) = delete;
  VBuffer &operator=(const VBuffer &) = delete;

  VBuffer(VBuffer &&other)
    : _device { other._device }
    , _buffer { std::move(other._buffer) }
    , _bufferInfo { std::move(other._bufferInfo) }
    , _bufferMemory { std::move(other._bufferMemory) }
  {
    other._device = nullptr;
    other._buffer = std::nullopt;
    other._bufferMemory = std::nullopt;
  };

  VBuffer &operator=(VBuffer &&other) {
    this->_device = std::move(other._device);
    this->_buffer = std::move(other._buffer);
    this->_bufferInfo = std::move(other._bufferInfo);
    this->_bufferMemory = std::move(other._bufferMemory);

    other._device = nullptr;
    other._buffer = std::nullopt;
    other._bufferMemory = std::nullopt;

    return *this;
  };

  ///// MAIN CONSTRUCTOR /////////////////////////////////////////////////////
  VBuffer(const vk::PhysicalDevice &physicalDevice, const vk::Device &device,
          size_t count, vk::BufferUsageFlags bufferUsageFlags,
          vk::MemoryPropertyFlags memoryPropertyFlags)
    : _device { &device }
  {
    _bufferInfo = { {}, sizeof(T) * count, bufferUsageFlags};
    _buffer = _device->createBuffer(_bufferInfo);

    auto vertexBufferMemReq = _device->getBufferMemoryRequirements(_buffer.value());
    vk::MemoryAllocateInfo vertexAllocInfo{
        vertexBufferMemReq.size,
        findMemoryType(physicalDevice, vertexBufferMemReq.memoryTypeBits,
                       memoryPropertyFlags)};

    _bufferMemory = device.allocateMemory(vertexAllocInfo);
    _device->bindBufferMemory(_buffer.value(), _bufferMemory.value(), 0);
  }

  ///// METHODS //////////////////////////////////////////////////////////////
  void* mapBufferMemory() {
    return _device->mapMemory(_bufferMemory.value(), 0, _bufferInfo.size);
  }

  void unmapBufferMemory() {
    _device->unmapMemory(_bufferMemory.value());
  }

  void transfer(std::vector<T> items) {
    auto mapped = mapBufferMemory();
      memcpy(mapped, items.data(), (size_t) _bufferInfo.size);
    unmapBufferMemory();
  }

  ///// GETTERS & SETTERS ////////////////////////////////////////////////////
  const vk::Buffer buffer() const {
    return _buffer.value();
  }

  ///// DESTRUCTOR ///////////////////////////////////////////////////////////
  ~VBuffer() {
    if(_buffer.has_value()) {
      _device->destroyBuffer(_buffer.value());
    }
    if(_bufferMemory.has_value()) {
      _device->freeMemory(_bufferMemory.value());
    }

  }

  private:
    const vk::Device* _device;
    vk::BufferCreateInfo _bufferInfo;
    std::optional<vk::Buffer> _buffer;
    std::optional<vk::DeviceMemory> _bufferMemory;


    uint32_t findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
      auto memProperties = physicalDevice.getMemoryProperties();
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
          if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
              return i;
          }
      }

      throw std::runtime_error("failed to find suitable memory type!");
    }
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
        float framebufferHeight,
        uint32_t MAX_FRAMES_IN_FLIGHT
    )
      : _physicalDevice { physicalDevice }
      , _device { device }
      , _framebufferWidth { framebufferWidth }
      , _framebufferHeight { framebufferHeight }
    {
      updateProjectionMatrix();

      std::cout << "device? " << device << std::endl;

      ///// DESCRIPTOR SET LAYOUT //////////////////////////////////////////////
      std::vector<vk::DescriptorSetLayoutBinding> uboLayoutBindings {
        {
          0, vk::DescriptorType::eUniformBuffer, 1
          , vk::ShaderStageFlagBits::eVertex
        }
      };

      vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo { 
        {}
        , uboLayoutBindings
      };

      _descriptorSetLayout = _device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

      ///// PIPELINE SETUP /////////////////////////////////////////////////////
      Shader vertexShader(&device, "src/shaders/simple.vert.bin");
      Shader fragmentShader(&device, "src/shaders/simple.frag.bin");
      _pipeline = GraphicsPipeline<Vertex>(
        device,
        renderPass,
        std::move(vertexShader),
        std::move(fragmentShader),
        { _descriptorSetLayout }
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

      _vertexBuffer = VBuffer<Vertex>(physicalDevice, device, _vertices.size()
          , vk::BufferUsageFlagBits::eVertexBuffer
          , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      
      _vertexBuffer.transfer(_vertices);

      ///// INDEX BUFFER SETUP /////////////////////////////////////////////////
      _indexBuffer = VBuffer<uint32_t>(physicalDevice, device, _indices.size()
        , vk::BufferUsageFlagBits::eIndexBuffer
        , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

      _indexBuffer.transfer(_indices);

      ///// CREATE UNIFORM BUFFERS /////////////////////////////////////////////
      for(auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto buff = VBuffer<UniformBufferObject>(
          _physicalDevice
          , _device
          , 1
          , vk::BufferUsageFlagBits::eUniformBuffer
          , vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        _uniformBuffersMapped.push_back(buff.mapBufferMemory());
        _uniformBuffers.push_back(std::move(buff));
      }

      ///// CREATE DESCRIPTOR POOL /////////////////////////////////////////////
      std::vector<vk::DescriptorPoolSize> descriptorPoolSizes {
        {
          vk::DescriptorType::eUniformBuffer
          , MAX_FRAMES_IN_FLIGHT
        }
      };

      vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {
        {}
        , MAX_FRAMES_IN_FLIGHT
        , descriptorPoolSizes
      };

      _descriptorPool = _device.createDescriptorPool(descriptorPoolCreateInfo);

      ///// CREATE DESCRIPTORS /////////////////////////////////////////////////
      std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);

      vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo {
        _descriptorPool, layouts
      };

      _descriptorSets = _device.allocateDescriptorSets(descriptorSetAllocateInfo);

      ///// CONFIGURE DESCRIPTORS //////////////////////////////////////////////
      for(auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo { 
          _uniformBuffers[i].buffer(), 0, sizeof(UniformBufferObject)
        };

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets {
          {
            _descriptorSets[i], 0, 0, 1
            , vk::DescriptorType::eUniformBuffer
            , {}, &bufferInfo, {}
          }
        };

        _device.updateDescriptorSets(writeDescriptorSets, {});
      }

    }


    ///// DESTRUCTOR ///////////////////////////////////////////////////////////
    ~Foxtalk() {
      _device.destroyDescriptorPool(_descriptorPool);
      _device.destroyDescriptorSetLayout(_descriptorSetLayout);
    }

    ///// DRAWING //////////////////////////////////////////////////////////////
    void tick() { }

    void render(
        const vk::CommandBuffer& commandBuffer
        , const vk::Extent2D swapchainExtent
        , uint32_t imageIndex
    ) {
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

      UniformBufferObject ubo{};
      ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      ubo.proj = glm::perspective(glm::radians(45.0f), _framebufferWidth / (float) _framebufferHeight, 0.1f, 10.0f);
      /* ubo.proj[1][1] *= -1; */

      memcpy(_uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));

      commandBuffer.setScissor(0, scissors);

      commandBuffer.bindVertexBuffers(0, _vertexBuffer.buffer(), { 0 });
      commandBuffer.bindIndexBuffer(_indexBuffer.buffer(), 0, vk::IndexType::eUint32);;
      commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics
        , _pipeline.layout()
        , 0
        , _descriptorSets[imageIndex]
        , {}
      );
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
    VBuffer<Vertex> _vertexBuffer;

    std::vector<uint32_t> _indices;
    VBuffer<uint32_t> _indexBuffer;

    std::vector<VBuffer<UniformBufferObject>> _uniformBuffers;
    std::vector<void*> _uniformBuffersMapped;

    vk::DescriptorSetLayout _descriptorSetLayout;
    vk::DescriptorPool _descriptorPool;
    std::vector<vk::DescriptorSet> _descriptorSets;

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
