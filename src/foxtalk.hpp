//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

// TODO: Wrap in linux preprocessors...
#include <dlfcn.h>
#include <exception>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <emmintrin.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/freetype.hpp>

#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <optional>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "boot/shader.hpp"
#include "boot/GraphicsPipeline.hpp"
#include "Vertex.hpp"

////////////////////////////////////////////////////////////////////////////////

/* constexpr const char* image_proc_file = "./libimage_proc.so"; */
constexpr const char* image_proc_file = "./libdot_sandbox.so";

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
  public:
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

  ///// NO COPY ONLY MOVE BARK BARK //////////////////////////////////////////
  VBuffer() { }

  VBuffer(const VBuffer &) = delete;
  VBuffer &operator=(const VBuffer &) = delete;

  VBuffer(VBuffer &&other)
    : _device { other._device }
    , _buffer { std::move(other._buffer) }
    , _bufferInfo { std::move(other._bufferInfo) }
    , _bufferMemory { std::move(other._bufferMemory) }
    , _mappedMemory { std::move(other._mappedMemory) }
  {
    other._device = nullptr;
    other._buffer = std::nullopt;
    other._bufferMemory = std::nullopt;
    other._mappedMemory = std::nullopt;
  };

  VBuffer &operator=(VBuffer &&other) {
    this->_device = std::move(other._device);
    this->_buffer = std::move(other._buffer);
    this->_bufferInfo = std::move(other._bufferInfo);
    this->_bufferMemory = std::move(other._bufferMemory);
    this->_mappedMemory = std::move(other._mappedMemory);

    other._device = nullptr;
    other._buffer = std::nullopt;
    other._bufferMemory = std::nullopt;
    other._mappedMemory = std::nullopt;

    return *this;
  };


  ///// METHODS //////////////////////////////////////////////////////////////
  void* mapBufferMemory() {
    if(_mappedMemory.has_value()) {
      return _mappedMemory.value();
    }

    _mappedMemory = _device->mapMemory(_bufferMemory.value(), 0, _bufferInfo.size);
    return _mappedMemory.value();
  }

  void unmapBufferMemory() {
    _mappedMemory = std::nullopt;
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
      if(_mappedMemory.has_value()) {
        _device->unmapMemory(_bufferMemory.value());
      }
      _device->freeMemory(_bufferMemory.value());
    }

  }

  private:
    const vk::Device* _device;
    vk::BufferCreateInfo _bufferInfo;
    std::optional<vk::Buffer> _buffer;
    std::optional<vk::DeviceMemory> _bufferMemory;
    std::optional<void*> _mappedMemory;


};

////////////////////////////////////////////////////////////////////////////////
struct VImage {
  ///// MAIN CONSTRUCTOR /////////////////////////////////////////////////////
  // N.B. Any of the `vk::*` objects are a point of possible abstraction.
  // In this case, Vulkan is *so* flexible that it's kind of unweildy for
  // the admittedly narrow case I'm using it for here. This object clumps
  // together:
  //   * The Image
  //   * The Image's Memory
  //   * The ImageView
  //   * The ImageSampler
  //
  // Essentially, the assumption here is that I'm going to load a texture,
  // and this texture will be sent to the GPU to be used in the shader pipeline.
  // This is a safe assumption, but certainly doesn't cover the full power or
  // range of what Vulkan has to offer.
  VImage(const vk::PhysicalDevice &physicalDevice
      , const vk::Device &device
      , uint32_t width
      , uint32_t height
  )
    : _device { &device }
    , _width { width }
    , _height { height }
    , _imageFormat { vk::Format::eB8G8R8A8Srgb }
  {
    ///// IMAGE //////////////////////////////////////////////////////////////
    vk::ImageCreateInfo imageCreateInfo {
      {}
      , vk::ImageType::e2D
      , _imageFormat
      , { _width, _height, 1 } // TODO Get real image size!!!
      , 1, 1
      , vk::SampleCountFlagBits::e1
      , vk::ImageTiling::eOptimal
      , vk::ImageUsageFlagBits::eTransferDst
        | vk::ImageUsageFlagBits::eSampled
      , {}, {}, {}
      , vk::ImageLayout::eUndefined
    };

    _image = _device->createImage(imageCreateInfo);

    ///// IMAGE MEMORY ///////////////////////////////////////////////////////
    auto imageMemoryReqs = _device->getImageMemoryRequirements(_image.value());

    vk::MemoryAllocateInfo imageMemoryAllocInfo {
      imageMemoryReqs.size,
        findMemoryType(physicalDevice, imageMemoryReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    };

    _imageMemory = _device->allocateMemory(imageMemoryAllocInfo);
    _device->bindImageMemory(_image.value(), _imageMemory.value(), 0);

    ///// IMAGE VIEW /////////////////////////////////////////////////////////
    vk::ImageViewCreateInfo imageViewCreateInfo {
      {}
      , _image.value()
      , vk::ImageViewType::e2D
      , _imageFormat
      , {}
      , {
        vk::ImageAspectFlagBits::eColor
        , 0, 1
        , 0, 1
      }
    };

    _imageView = _device->createImageView(imageViewCreateInfo);

    ///// SAMPLER ////////////////////////////////////////////////////////////

    auto physicalDeviceProperties = physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerCreateInfo {
      {}
      , vk::Filter::eLinear
      , vk::Filter::eLinear
      , vk::SamplerMipmapMode::eLinear, {}, {}, {}
      , 0.0
      , vk::True
      , physicalDeviceProperties.limits.maxSamplerAnisotropy
      , vk::False, vk::CompareOp::eAlways
      , 0.0f, 0.0
      , vk::BorderColor::eIntOpaqueBlack
    };
    // I ran out of LSP info in vim, so I have no idea where this
    // parameter actuall is <_<
    samplerCreateInfo.setUnnormalizedCoordinates(vk::False);

    _imageSampler = _device->createSampler(samplerCreateInfo);
  }

  ///// NO COPY! /////////////////////////////////////////////////////////////
  VImage(const VImage &) = delete;
  VImage &operator=(const VImage &) = delete;

  ///// ONLY MOVE ////////////////////////////////////////////////////////////
  VImage(VImage &&other)
    : _device       { std::move(other._device) }
    , _image        { std::move(other._image) }
    , _imageMemory  { std::move(other._imageMemory) }
    , _imageView    { std::move(other._imageView) }
    , _imageLayout  { std::move(other._imageLayout) }
    , _imageSampler { std::move(other._imageSampler) }
    , _imageFormat  { std::move(other._imageFormat) }
    , _width        { std::move(other._width) }
    , _height       { std::move(other._height) }
  {
    other._device       = nullptr;
    other._image        = std::nullopt;
    other._imageMemory  = std::nullopt;
    other._imageView    = std::nullopt;
    other._imageSampler = std::nullopt;
  }

  VImage &operator=(VImage &&other) {
    this->_device       = std::move(other._device);
    this->_image        = std::move(other._image);
    this->_imageMemory  = std::move(other._imageMemory);
    this->_imageView    = std::move(other._imageView);
    this->_imageSampler = std::move(other._imageSampler);
    this->_imageLayout  = std::move(other._imageLayout);
    this->_imageFormat  = std::move(other._imageFormat);
    this->_width        = std::move(other._width);
    this->_height       = std::move(other._height);

    other._device       = nullptr;
    other._image        = std::nullopt;
    other._imageMemory  = std::nullopt;
    other._imageView    = std::nullopt;
    other._imageSampler = std::nullopt;

    return *this;
  }


  ///// METHODS //////////////////////////////////////////////////////////////
  void transitionImageLayout(const vk::CommandBuffer& cmdBuffer, vk::ImageLayout newLayout) {
    if(newLayout == _imageLayout) return;

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
    // Shader Reading --> Transfer Dest
    else if(_imageLayout == vk::ImageLayout::eReadOnlyOptimal
        && newLayout == vk::ImageLayout::eTransferDstOptimal
    ){
      srcAccessFlags = vk::AccessFlagBits::eNone;
      dstAccessFlags = vk::AccessFlagBits::eTransferWrite;

      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
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
  ///// GETTERS & SETTERS ////////////////////////////////////////////////////
  const vk::Sampler sampler() const {
    return _imageSampler.value();
  }

  const vk::ImageView imageView() const {
    return _imageView.value();
  }

  const vk::ImageLayout imageLayout() const {
    return _imageLayout;
  }

  const vk::Format format() const {
    return _imageFormat;
  }

  ///// DESTRUCTOR ///////////////////////////////////////////////////////////
  ~VImage() {
    std::cout << "TMP DEBUG! VImage Destructor Called... ";
    if(_device != nullptr) {
      std::cout << "DESTROYING!" << std::endl;

      if(_imageSampler.has_value()) {
        _device->destroySampler(_imageSampler.value());
      } else {
        std::cerr << "WARNING! VImage had a _device, but not an _imageSampler!!!!" << std::endl;
      }
      _device->destroyImageView(_imageView.value());
      _device->destroyImage(_image.value());
      _device->freeMemory(_imageMemory.value());
    } else {
      std::cout << "I've been moved!" << std::endl;
    }

  }


  private:
    const vk::Device* _device;
    uint32_t _width, _height;


    std::optional<vk::Image> _image;
    std::optional<vk::DeviceMemory> _imageMemory;
    std::optional<vk::ImageView> _imageView;
    std::optional<vk::Sampler> _imageSampler;

    vk::Format _imageFormat;
    vk::ImageLayout _imageLayout = vk::ImageLayout::eUndefined;

};

////////////////////////////////////////////////////////////////////////////////

typedef void (*ExternalImageProc)(
  cv::Mat& cameraFrame
  , cv::Mat& outputFrame
  , cv::freetype::FreeType2* _cv_ft2
);


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

      ///// LOAD THE MAGIC LIBRARY /////////////////////////////////////////////

      _updateProc_handle = dlopen(image_proc_file, RTLD_NOW);
      if(_updateProc_handle == nullptr) {
        throw std::runtime_error("Could not load image_proc_file!");
      }

      _updateProc = (ExternalImageProc)dlsym(_updateProc_handle, "process_image");

      if(_updateProc == nullptr) {
        throw std::runtime_error("Could not load updateProc!");
      }

      ///// (CV2) FREETYPE INITIALIZATION //////////////////////////////////////
      _cv_ft2 = cv::freetype::createFreeType2();
      _cv_ft2->loadFontData("/usr/share/fonts/OTF/CascadiaCode-Regular.otf", 0);

      ///// VIDEO CAPTURE //////////////////////////////////////////////////////

      //--- INITIALIZE VIDEOCAPTURE
      // open the default camera using default API
      // cap.open(0);
      // OR advance usage: select any API backend
      int deviceID = 0;             // 0 = open default camera
      int apiID = cv::CAP_V4L2;     // 0 = autodetect default API
      // open selected camera using selected API
      _videoCapture.open(deviceID, apiID);
      // check if we succeeded
      if (!_videoCapture.isOpened()) {
        throw std::runtime_error("ERROR! Unable to open camera");
      }

      _camWidth = _videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
      _camHeight = _videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);

      std::cout << "Found camera with res " << _camWidth << "x" << _camHeight << std::endl;

      auto imageSize = _camWidth * _camHeight * 4;

      ///// DESCRIPTOR SET LAYOUT //////////////////////////////////////////////
      std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings {
        // Uniform Buffer
        vk::DescriptorSetLayoutBinding {
          0
          , vk::DescriptorType::eUniformBuffer
          , 1
          , vk::ShaderStageFlagBits::eVertex
        },
        // Camera Image Sampler
        vk::DescriptorSetLayoutBinding {
          1
          , vk::DescriptorType::eCombinedImageSampler
          , 1
          , vk::ShaderStageFlagBits::eFragment
        }
      };

      vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
        {}
        , descriptorSetLayoutBindings
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

      /* auto width = 1920.0f; */
      auto width = 3839.0f;
      auto offset = 0.0f; //10.0f;

      auto height = width * (static_cast<float>(_camHeight) / _camWidth);

      _vertices = {
          {{offset,          offset},         {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
          {{offset,          height + offset}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
          {{width + offset, height + offset}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
          {{width + offset, offset},         {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}
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

        VImage camImage {
          _physicalDevice
            , _device
            , _camWidth
            , _camHeight
        };

        _cameraBuffers.push_back(std::move(buff));
        _cameraImages.push_back(std::move(camImage));
      }


      ///// CREATE DESCRIPTOR POOL /////////////////////////////////////////////
      std::vector<vk::DescriptorPoolSize> descriptorPoolSizes {
        vk::DescriptorPoolSize {
          vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT
        },
        vk::DescriptorPoolSize {
          vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT
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

        auto& img = _cameraImages[i];
        vk::DescriptorImageInfo imageInfo {
          img.sampler()
          , img.imageView()
          // TODO: Hmm... I wonder why this isn't the same layout as the actual image?
          , vk::ImageLayout::eShaderReadOnlyOptimal
        };

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets {
          vk::WriteDescriptorSet {
            _descriptorSets[i], 0, 0, 1
            , vk::DescriptorType::eUniformBuffer
            , {}, &bufferInfo, {}
          },
          vk::WriteDescriptorSet {
            _descriptorSets[i], 1, 0, 1,
            vk::DescriptorType::eCombinedImageSampler
            , &imageInfo, {}, {}
          }
        };

        _device.updateDescriptorSets(writeDescriptorSets, {});
      }

    }

    ///// DESTRUCTOR ///////////////////////////////////////////////////////////
    ~Foxtalk() {
      dlclose(_updateProc_handle);

      _device.destroyDescriptorPool(_descriptorPool);
      _device.destroyDescriptorSetLayout(_descriptorSetLayout);
    }

    ///// DRAWING //////////////////////////////////////////////////////////////
    void tick() { }

    struct stat fstat_buf {};
    struct timespec last_changed {};
    int fd;
    bool needsReload = false;

    void updateCameraTextureBuffer(VBuffer<uint8_t>& cameraBuffer) {
      ///// Check for code changes (This probably doesn't have to be done every frame :grimace:

      fd = open(image_proc_file, O_RDONLY);
      fstat(fd, &fstat_buf);
      close(fd);

      if(last_changed.tv_sec != fstat_buf.st_mtim.tv_sec) {
        last_changed = fstat_buf.st_mtim;

        std::cout << "Saw a change!!!" << std::endl;
        needsReload = true;
        _updateProc = nullptr;
        dlclose(_updateProc_handle);
      }

      if(needsReload) {
        _updateProc_handle = dlopen(image_proc_file, RTLD_NOW);
        if(_updateProc_handle != nullptr) {
          _updateProc = (ExternalImageProc)dlsym(_updateProc_handle, "process_image");

          if(_updateProc == nullptr) {
            throw std::runtime_error("Could not load updateProc!");
          }

          needsReload = false;
        }
      }


      ///// Run the frame...
      cv::Mat cameraFrame;

      _videoCapture.read(cameraFrame);
      // check if we succeeded
      if (cameraFrame.empty()) {
        throw std::runtime_error("ERROR! blank frame grabbed");
      }

      auto rot_mat = cv::getRotationMatrix2D(
          { static_cast<float>(_camWidth) / 2.0f, static_cast<float>(_camHeight) / 2.0f }
          , 180.0
          , 1.0);

      cv::warpAffine(cameraFrame, cameraFrame, rot_mat, cameraFrame.size());

      cv::Mat outputMat = cv::Mat::ones(_camWidth, _camHeight, cameraFrame.type()) * 255;

      if(_updateProc != nullptr) {
        _updateProc(
          cameraFrame
	  , outputMat
          , _cv_ft2
        );
      }

      // Add filled alpha channel, since Vulkan drivers seem to not support
      // alphaless textures
      //
      std::vector<cv::Mat> channels;
      cv::split(outputMat, channels);
      cv::Mat alphaChannel = cv::Mat::ones(cameraFrame.size(), CV_8UC1) * 255;
      channels.push_back(alphaChannel);


      cv::Mat finalOutputMat;
      cv::merge(channels, finalOutputMat);


      // TODO: SYNC---Handled by sync2 extension??

      // Write camera data
      // TODO: Image Size!

      memcpy(cameraBuffer.mapBufferMemory(), finalOutputMat.data, static_cast<size_t>(_camWidth * _camHeight * 4));

      // TODO: SYNC---Handled by sync2 extension??
    }

    void render(
        const vk::CommandBuffer& commandBuffer
        , const vk::RenderPassBeginInfo& renderPassBeginInfo
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

      // TODO: COPY IMAGE DATA INTO MAPPED BUFFER
      camImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);

      updateCameraTextureBuffer(camBuffer);
      camImage.copyBufferToImage(commandBuffer, camBuffer);

      camImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eReadOnlyOptimal);

      ///// BEGIN RENDERPASS ///////////////////////////////////////////////////

      commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

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

      ///// END RENDERPASS /////////////////////////////////////////////////////

      commandBuffer.endRenderPass();
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

    void* _updateProc_handle;
    ExternalImageProc _updateProc;
    int _inotify_fd;

    cv::VideoCapture _videoCapture;
    cv::Ptr<cv::freetype::FreeType2> _cv_ft2;

    uint32_t _camWidth;
    uint32_t _camHeight;

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
