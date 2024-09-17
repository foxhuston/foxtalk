//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "boot/shader.hpp"
#include "boot/GraphicsPipeline.hpp"
#include "Vertex.hpp"

////////////////////////////////////////////////////////////////////////////////

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

////////////////////////////////////////////////////////////////////////////////

static uint32_t findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  auto memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
          return i;
      }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

////////////////////////////////////////////////////////////////////////////////

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


};

////////////////////////////////////////////////////////////////////////////////
struct VImage {
  ///// NO COPY! /////////////////////////////////////////////////////////////
  VImage(const VImage &) = default;
  VImage &operator=(const VImage &) = default;

  ///// ONLY MOVE ////////////////////////////////////////////////////////////
  VImage(VImage &&other)
    : _device      { std::move(other._device) }
    , _image       { std::move(other._image) }
    , _imageMemory { std::move(other._imageMemory) }
    , _imageLayout { other._imageLayout }
    , _imageFormat { other._imageFormat }
    , _width       { other._width }
    , _height      { other._height }
  {
    other._device      = nullptr;
    other._image       = std::nullopt;
    other._imageMemory = std::nullopt;
  }

  VImage &operator=(VImage &&other) {
    this->_device      = other._device;
    this->_image       = other._image;
    this->_imageMemory = other._imageMemory;
    this->_imageLayout = other._imageLayout;
    this->_imageFormat = other._imageFormat;
    this->_width       = other._width;
    this->_height      = other._height;

    other._device      = nullptr;
    other._image       = std::nullopt;
    other._imageMemory = std::nullopt;
    return *this;
  }

  ///// MAIN CONSTRUCTOR /////////////////////////////////////////////////////
  VImage(const vk::PhysicalDevice &physicalDevice, const vk::Device &device)
    : _device { &device }
    , _width { 640 }
    , _height { 480 }
    , _imageFormat { vk::Format::eB8G8R8A8Srgb }
  {
    vk::ImageCreateInfo imageCreateInfo {
      {}
      , vk::ImageType::e2D
      , _imageFormat
      , { _width, _height, 1 }             // TODO Get real image size!!!
      , 1 , 1 , {}, {}
      , vk::ImageUsageFlagBits::eTransferDst
        | vk::ImageUsageFlagBits::eSampled
      , {}, {}, {}
      , vk::ImageLayout::eUndefined
    };

    _image = _device->createImage(imageCreateInfo);

    auto imageMemoryReqs = _device->getImageMemoryRequirements(_image.value());

    vk::MemoryAllocateInfo imageMemoryAllocInfo {
      imageMemoryReqs.size,
        findMemoryType(physicalDevice, imageMemoryReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    };

    _imageMemory = _device->allocateMemory(imageMemoryAllocInfo);
    _device->bindImageMemory(_image.value(), _imageMemory.value(), 0);
  }

  ///// METHODS //////////////////////////////////////////////////////////////
  void transitionImageLayout(const vk::CommandBuffer& cmdBuffer, vk::ImageLayout newLayout) {
    vk::AccessFlags srcAccessFlags       = vk::AccessFlagBits::eNone;
    vk::AccessFlags dstAccessFlags       = vk::AccessFlagBits::eNone;
    vk::PipelineStageFlags srcPipelineStageFlags = vk::PipelineStageFlagBits::eNone;
    vk::PipelineStageFlags dstPipelineStageFlags = vk::PipelineStageFlagBits::eNone;


    // Undefined --> Transfer Dest
    if(_imageLayout == vk::ImageLayout::eUndefined
        && newLayout == vk::ImageLayout::eTransferDstOptimal
    ) {
      dstAccessFlags = vk::AccessFlagBits::eTransferWrite;
      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    // Transfer Dest --> Shader Reading
    else if(_imageLayout == vk::ImageLayout::eTransferDstOptimal
        && newLayout == vk::ImageLayout::eReadOnlyOptimal
    ){
      srcAccessFlags = vk::AccessFlagBits::eTransferWrite;
      dstAccessFlags = vk::AccessFlagBits::eShaderRead;

      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    // Unknown Op.
    else {
      throw std::invalid_argument("unsupported layout transition!");
    }


    vk::ImageMemoryBarrier barrier {
      srcAccessFlags
      , dstAccessFlags
      , _imageLayout
      , newLayout
      , vk::QueueFamilyIgnored
      , vk::QueueFamilyIgnored
      , _image.value()
      , {
        vk::ImageAspectFlagBits::eColor
        , 0, 1, 0, 1
      }
    };

    _imageLayout = newLayout;

    cmdBuffer.pipelineBarrier(
      srcPipelineStageFlags
      , dstPipelineStageFlags
      , vk::DependencyFlagBits::eByRegion
      , {}, {}, barrier
    );
  }

  void copyBufferToImage(const vk::CommandBuffer& cmdBuffer, const VBuffer<uint8_t> &buffer) const {
    vk::BufferImageCopy bufferImageCopy {
      0, 0, 0
      , {
        vk::ImageAspectFlagBits::eColor
        , 0, 0, 1
      }
      , {0, 0, 0}
      , { _width, _height, 1 }
    };


    cmdBuffer.copyBufferToImage(
        buffer.buffer(), _image.value()
        , vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
  };

  ///// DESTRUCTOR ///////////////////////////////////////////////////////////
  ~VImage() {
    if(_device != nullptr) {
      _device->destroyImage(_image.value());
      _device->freeMemory(_imageMemory.value());
    }
  }


  private:
    const vk::Device* _device;
    uint32_t _width, _height;

    vk::ImageLayout _imageLayout = vk::ImageLayout::eUndefined;
    vk::Format _imageFormat;

    std::optional<vk::Image> _image;
    std::optional<vk::DeviceMemory> _imageMemory;
    
};

////////////////////////////////////////////////////////////////////////////////

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
      ///// VIDEO CAPTURE //////////////////////////////////////////////////////

      int texWidth = 640, texHeight = 480, texChannels = 3;
      auto imageSize = texWidth * texHeight * 4;

      //--- INITIALIZE VIDEOCAPTURE
      // open the default camera using default API
      // cap.open(0);
      // OR advance usage: select any API backend
      int deviceID = 0;             // 0 = open default camera
      int apiID = cv::CAP_ANY;      // 0 = autodetect default API
      // open selected camera using selected API
      _videoCapture.open(deviceID, apiID);
      // check if we succeeded
      if (!_videoCapture.isOpened()) {
        throw std::runtime_error("ERROR! Unable to open camera");
      }
      
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
          {{10.0f, 10.0f}, {1.0f, 0.0f, 0.0f}},
          {{10.0f, 100.0f}, {0.0f, 1.0f, 0.0f}},
          {{100.0f, 100.0f}, {0.0f, 0.0f, 1.0f}},
          {{100.0f, 10.0f}, {1.0f, 0.0f, 0.0f}}
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

      ///// CREATE VIDEO IMAGES & BUFFERS //////////////////////////////////////
      for(auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto buff = VBuffer<uint8_t>(
          _physicalDevice
          , _device
          , imageSize
          , vk::BufferUsageFlagBits::eTransferSrc
          , vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        _cameraBuffersMapped.push_back(buff.mapBufferMemory());
        _cameraBuffers.push_back(std::move(buff));

        VImage vimg { _physicalDevice, _device };
        _cameraImages.push_back(vimg);
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
      ///// SET UP PIPELINE ////////////////////////////////////////////////////
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

      ///// CAPTURE CAM IMAGE //////////////////////////////////////////////////
      // TODO: This should probably *not* be done in the render loop??

      auto& camImage        = _cameraImages[imageIndex];
      auto& camBuffer       = _cameraBuffers[imageIndex];
      auto  camMemoryMapped = _cameraBuffersMapped[imageIndex];

      // TODO: COPY IMAGE DATA INTO MAPPED BUFFER
      camImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);
      camImage.copyBufferToImage(commandBuffer, camBuffer);

      ///// SET UP UBO /////////////////////////////////////////////////////////

      UniformBufferObject ubo{};
      ubo.model = glm::mat4(1.0f); //glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      ubo.view = glm::lookAt(
          glm::vec3(0.0f, 0.001f, 3.0f)
          , glm::vec3(0.0f, 0.0f, 0.0f)
          , glm::vec3(0.0f, 0.0f, 1.0f));

      ubo.view = glm::mat4(1.0f);
      ubo.proj = glm::ortho(0.0f, _framebufferWidth, _framebufferHeight, 0.0f);


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

    cv::VideoCapture _videoCapture;

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

    std::vector<VBuffer<uint8_t>> _cameraBuffers;
    std::vector<void*> _cameraBuffersMapped;
    std::vector<VImage> _cameraImages;

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
