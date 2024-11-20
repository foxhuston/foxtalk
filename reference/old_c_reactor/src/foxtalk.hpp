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

#include "Reactor.h"

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "boot/shader.hpp"
#include "boot/GraphicsPipeline.hpp"
#include "Vertex.hpp"
#include "VBuffer.h"
#include "VImage.h"

////////////////////////////////////////////////////////////////////////////////

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

class Foxtalk {
private:
  foxtalk::Reactor& _reactor;

  const vk::Device &_device;
  const vk::PhysicalDevice &_physicalDevice;

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
  std::vector<void *> _uniformBuffersMapped;

  std::vector<VBuffer<uint8_t>> _cameraBuffers;
  std::vector<VImage> _cameraImages;

  vk::DescriptorSetLayout _descriptorSetLayout;
  vk::DescriptorPool _descriptorPool;
  std::vector<vk::DescriptorSet> _descriptorSets;

  // Orthorgraphic.
  void updateProjectionMatrix() {
    _projection = {
        {2 / _framebufferWidth, 0.0f,                   0.0f,               -1.0f},
        {0.0f,                  2 / _framebufferHeight, 0.0f,               -1.0f},
        {0.0f,                  0.0f,                   2 / (_far - _near), 0.0f},
        {0.0f,                  0.0f,                   0.0f,               1.0f}
    };
  }

public:
  ///// CONSTRUCTOR //////////////////////////////////////////////////////////
  Foxtalk(
      foxtalk::Reactor& reactor,
      // TODO: Needing both the device & physicacalDevice here feels like a
      //       leaky abstraction...
      const vk::PhysicalDevice &physicalDevice,
      const vk::Device &device,
      const vk::RenderPass &renderPass,
      float framebufferWidth,
      float framebufferHeight,
      uint32_t maxFramesInFlight
  )
      : _reactor{reactor}, _physicalDevice{physicalDevice}, _device{device}, _framebufferWidth{framebufferWidth},
        _framebufferHeight{framebufferHeight} {
    updateProjectionMatrix();

    std::cout << "device? " << device << std::endl;

    ///// LOAD THE MAGIC LIBRARY /////////////////////////////////////////////
    // TODO

    ///// (CV2) FREETYPE INITIALIZATION //////////////////////////////////////
    _cv_ft2 = cv::freetype::createFreeType2();
    _cv_ft2->loadFontData("/usr/share/fonts/OTF/CascadiaCode-Regular.otf", 0);

    ///// VIDEO CAPTURE //////////////////////////////////////////////////////

    //--- INITIALIZE VIDEOCAPTURE
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_V4L2;     //
    // open selected camera using selected API
    _videoCapture.open(deviceID, apiID);
    // check if we succeeded
    if (!_videoCapture.isOpened()) {
      throw std::runtime_error("ERROR! Unable to open camera");
    }

    _videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1920);
    _videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 1080);
    _videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, 60);

    _camWidth = _videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
    _camHeight = _videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);

    std::cout << "Found camera with res " << _camWidth << "x" << _camHeight << std::endl;

    auto imageSize = _camWidth * _camHeight * 4;

    ///// DESCRIPTOR SET LAYOUT //////////////////////////////////////////////
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings{
        // Uniform Buffer
        vk::DescriptorSetLayoutBinding{
            0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex
        },
        // Camera Image Sampler
        vk::DescriptorSetLayoutBinding{
            1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment
        }
    };

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
        {}, descriptorSetLayoutBindings
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
        {_descriptorSetLayout}
    );

    ///// TEMP DATA //////////////////////////////////////////////////////////

    auto width = 1920; //1280.0f;
//      auto width = 3839.0f; // What. Projector?
    auto offset = 0.0f; //10.0f;

    auto height = width * (static_cast<float>(_camHeight) / _camWidth);

    _vertices = {
        {{offset,         offset},          {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{offset,         height + offset}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{width + offset, height + offset}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{width + offset, offset},          {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}
    };

    _indices = {
        0, 1, 2, 2, 3, 0
    };

    ///// VERTEX BUFFER SETUP ////////////////////////////////////////////////

    _vertexBuffer = VBuffer<Vertex>(physicalDevice, &device, _vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer,
                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent);

    _vertexBuffer.transfer(_vertices);

    ///// INDEX BUFFER SETUP /////////////////////////////////////////////////
    _indexBuffer = VBuffer<uint32_t>(physicalDevice, &device, _indices.size(), vk::BufferUsageFlagBits::eIndexBuffer,
                                     vk::MemoryPropertyFlagBits::eHostVisible |
                                     vk::MemoryPropertyFlagBits::eHostCoherent
    );

    _indexBuffer.transfer(_indices);

    ///// CREATE UNIFORM BUFFERS /////////////////////////////////////////////
    for (auto i = 0; i < maxFramesInFlight; i++) {
      auto buff = VBuffer<UniformBufferObject>(
          _physicalDevice, &_device, 1, vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible
          | vk::MemoryPropertyFlagBits::eHostCoherent
      );

      _uniformBuffersMapped.push_back(buff.mapBufferMemory());
      _uniformBuffers.push_back(std::move(buff));
    }

    ///// CREATE VIDEO IMAGES & BUFFERS //////////////////////////////////////
    for (auto i = 0; i < maxFramesInFlight; i++) {
      auto buff = VBuffer<uint8_t>(
          _physicalDevice, &_device, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible
          | vk::MemoryPropertyFlagBits::eHostCoherent
      );

      VImage camImage{
          _physicalDevice, _device, _camWidth, _camHeight
      };

      _cameraBuffers.push_back(std::move(buff));
      _cameraImages.push_back(std::move(camImage));
    }


    ///// CREATE DESCRIPTOR POOL /////////////////////////////////////////////
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes{
        vk::DescriptorPoolSize{
            vk::DescriptorType::eUniformBuffer, maxFramesInFlight
        },
        vk::DescriptorPoolSize{
            vk::DescriptorType::eCombinedImageSampler, maxFramesInFlight
        }
    };

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{
        {}, maxFramesInFlight, descriptorPoolSizes
    };

    _descriptorPool = _device.createDescriptorPool(descriptorPoolCreateInfo);

    ///// CREATE DESCRIPTORS /////////////////////////////////////////////////
    std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, _descriptorSetLayout);

    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
        _descriptorPool, layouts
    };

    _descriptorSets = _device.allocateDescriptorSets(descriptorSetAllocateInfo);

    ///// CONFIGURE DESCRIPTORS //////////////////////////////////////////////
    for (auto i = 0; i < maxFramesInFlight; i++) {
      vk::DescriptorBufferInfo bufferInfo{
          _uniformBuffers[i].buffer(), 0, sizeof(UniformBufferObject)
      };

      auto &img = _cameraImages[i];
      vk::DescriptorImageInfo imageInfo{
          img.sampler(), img.imageView()
          // TODO: Hmm... I wonder why this isn't the same layout as the actual image?
          , vk::ImageLayout::eShaderReadOnlyOptimal
      };

      std::vector<vk::WriteDescriptorSet> writeDescriptorSets{
          vk::WriteDescriptorSet{
              _descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, {}, &bufferInfo, {}
          },
          vk::WriteDescriptorSet{
              _descriptorSets[i], 1, 0, 1,
              vk::DescriptorType::eCombinedImageSampler, &imageInfo, {}, {}
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
  void tick() {}

  static void cvFree(void *ptr) {
//    std::cout << "cvFree! " << ptr << std::endl;
    delete static_cast<cv::Mat *>(ptr);
  }

  void updateCameraTextureBuffer(VBuffer<uint8_t> &cameraBuffer) {
    // remove any old frames.
    auto old_frames = _reactor.query(
        mkQuery(), mkSymbol("is a"), mkSymbol("camera frame"));
    _reactor.remove(old_frames);

    ///// Run the frame...
    cv::Mat cameraFrame;

    _videoCapture.read(cameraFrame);
    // check if we succeeded
    if (cameraFrame.empty()) {
      throw std::runtime_error("ERROR! blank frame grabbed");
    }

    auto rot_mat = cv::getRotationMatrix2D(
        {static_cast<float>(_camWidth) / 2.0f, static_cast<float>(_camHeight) / 2.0f}, 180.0, 1.0);

    cv::warpAffine(cameraFrame, cameraFrame, rot_mat, cameraFrame.size());

    // Insert new image.
    auto image_frame = new cv::Mat(cameraFrame);
    auto imagePtr = mkPtr(image_frame, cvFree);
    _reactor.claim(
        imagePtr,
        mkSymbol("is a"),
        mkSymbol("camera frame")
    );

    // TODO: This is here because removes are eager, while
    // evaluation happens every tick. I think I need to have
    // a marker for tuples / handlers that get removed, and do so
    // at the end of a tick, rather than eagerly.
    _reactor.tick(); // TODO: WOAH, CHEATING

    auto output_layers = _reactor.query(
        mkQuery(),
        mkSymbol("is a"),
        mkSymbol("output layer")
    );

    cv::Mat finalOutputMat;
    if(!output_layers.empty()) {
      auto fst = *output_layers.begin();
//      std::cout << "Using " << *fst << std::endl;

      auto img = (cv::Mat *) fst->getSubject()->data.cptr.data;

      // Add filled alpha channel, since Vulkan drivers seem to not support
      // alphaless textures
      std::vector<cv::Mat> channels;
      cv::split(*img, channels);

      cv::Mat alphaChannel = cv::Mat::ones(cameraFrame.size(), CV_8UC1) * 255;
      channels.push_back(alphaChannel);

      cv::merge(channels, finalOutputMat);

      _reactor.remove(output_layers);
    } else {
      // Add filled alpha channel, since Vulkan drivers seem to not support
      // alphaless textures
      std::vector<cv::Mat> channels;
      cv::split(cameraFrame, channels);

      cv::Mat alphaChannel = cv::Mat::ones(cameraFrame.size(), CV_8UC1) * 127;
      channels.push_back(alphaChannel);

      cv::merge(channels, finalOutputMat);
    }


    // TODO: SYNC---Handled by sync2 extension??

    // Write camera data
    // TODO: Image Size!

    memcpy(cameraBuffer.mapBufferMemory(), finalOutputMat.data, static_cast<size_t>(_camWidth * _camHeight * 4));

    // TODO: SYNC---Handled by sync2 extension??
  }

  void render(
      const vk::CommandBuffer &commandBuffer, const vk::RenderPassBeginInfo &renderPassBeginInfo,
      const vk::Extent2D swapchainExtent, uint32_t imageIndex
  ) {
    ///// SET UP PIPELINE ////////////////////////////////////////////////////
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.pipeline());

    commandBuffer.setViewport(0, {{
                                      0.0f, 0.0f, static_cast<float>(swapchainExtent.width),
                                      static_cast<float>(swapchainExtent.height), 0.0f, 1.0f
                                  }});

    std::vector<vk::Rect2D> scissors{
        {
            {0, 0}, swapchainExtent
        }
    };

    ///// CAPTURE CAM IMAGE //////////////////////////////////////////////////
    // TODO: This should probably *not* be done in the render loop??

    auto &camImage = _cameraImages[imageIndex];
    auto &camBuffer = _cameraBuffers[imageIndex];

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
        glm::vec3(0.0f, 0.001f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = glm::mat4(1.0f);
    ubo.proj = glm::ortho(0.0f, _framebufferWidth, _framebufferHeight, 0.0f);


    /* ubo.proj[1][1] *= -1; */

    memcpy(_uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));

    commandBuffer.setScissor(0, scissors);

    commandBuffer.bindVertexBuffers(0, _vertexBuffer.buffer(), {0});
    commandBuffer.bindIndexBuffer(_indexBuffer.buffer(), 0, vk::IndexType::eUint32);;
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _pipeline.layout(), 0, _descriptorSets[imageIndex], {}
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

};

#endif // FOXTALK_FOXTALK_H
