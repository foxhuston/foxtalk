//
// Created by fox on 10/3/24.
//

#ifndef FOXTALK_VIMAGE_H
#define FOXTALK_VIMAGE_H

#include <iostream>
#include <vulkan/vulkan.hpp>
#include "VulkUtils.h"
#include "VBuffer.h"

class VImage {
private:
  const vk::Device *_device;
  uint32_t _width, _height;

  std::optional<vk::Image> _image;
  std::optional<vk::DeviceMemory> _imageMemory;
  std::optional<vk::ImageView> _imageView;
  std::optional<vk::Sampler> _imageSampler;

  vk::Format _imageFormat;
  vk::ImageLayout _imageLayout = vk::ImageLayout::eUndefined;

public:
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
  VImage(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, uint32_t width, uint32_t height)
      : _device{&device}, _width{width}, _height{height}, _imageFormat{vk::Format::eB8G8R8A8Srgb} {
    ///// IMAGE //////////////////////////////////////////////////////////////
    vk::ImageCreateInfo imageCreateInfo{
        {}, vk::ImageType::e2D, _imageFormat, {_width, _height, 1} // TODO Get real image size!!!
        , 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst
                                                                        | vk::ImageUsageFlagBits::eSampled, {}, {}, {},
        vk::ImageLayout::eUndefined
    };

    _image = _device->createImage(imageCreateInfo);

    ///// IMAGE MEMORY ///////////////////////////////////////////////////////
    auto imageMemoryReqs = _device->getImageMemoryRequirements(_image.value());

    vk::MemoryAllocateInfo imageMemoryAllocInfo{
        imageMemoryReqs.size,
        findMemoryType(physicalDevice, imageMemoryReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    };

    _imageMemory = _device->allocateMemory(imageMemoryAllocInfo);
    _device->bindImageMemory(_image.value(), _imageMemory.value(), 0);

    ///// IMAGE VIEW /////////////////////////////////////////////////////////
    vk::ImageViewCreateInfo imageViewCreateInfo{
        {}, _image.value(), vk::ImageViewType::e2D, _imageFormat, {}, {
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        }
    };

    _imageView = _device->createImageView(imageViewCreateInfo);

    ///// SAMPLER ////////////////////////////////////////////////////////////

    auto physicalDeviceProperties = physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerCreateInfo{
        {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, {}, {}, {}, 0.0, vk::True,
        physicalDeviceProperties.limits.maxSamplerAnisotropy, vk::False, vk::CompareOp::eAlways, 0.0f, 0.0,
        vk::BorderColor::eIntOpaqueBlack
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
  VImage(VImage &&other) noexcept
      : _device{other._device}, _image{other._image}, _imageMemory{other._imageMemory}, _imageView{other._imageView},
        _imageLayout{other._imageLayout}, _imageSampler{other._imageSampler}, _imageFormat{other._imageFormat},
        _width{other._width}, _height{other._height} {
    other._device = nullptr;
    other._image = std::nullopt;
    other._imageMemory = std::nullopt;
    other._imageView = std::nullopt;
    other._imageSampler = std::nullopt;
  }

  VImage &operator=(VImage &&other) noexcept {
    this->_device = other._device;
    this->_image = other._image;
    this->_imageMemory = other._imageMemory;
    this->_imageView = other._imageView;
    this->_imageSampler = other._imageSampler;
    this->_imageLayout = other._imageLayout;
    this->_imageFormat = other._imageFormat;
    this->_width = other._width;
    this->_height = other._height;

    other._device = nullptr;
    other._image = std::nullopt;
    other._imageMemory = std::nullopt;
    other._imageView = std::nullopt;
    other._imageSampler = std::nullopt;

    return *this;
  }

  ///// METHODS //////////////////////////////////////////////////////////////
  void transitionImageLayout(const vk::CommandBuffer &cmdBuffer, vk::ImageLayout newLayout) {
    if (newLayout == _imageLayout) return;

    vk::AccessFlags srcAccessFlags = vk::AccessFlagBits::eNone;
    vk::AccessFlags dstAccessFlags = vk::AccessFlagBits::eNone;
    vk::PipelineStageFlags srcPipelineStageFlags = vk::PipelineStageFlagBits::eNone;
    vk::PipelineStageFlags dstPipelineStageFlags = vk::PipelineStageFlagBits::eNone;


    // Undefined --> Transfer Dest
    if (_imageLayout == vk::ImageLayout::eUndefined
        && newLayout == vk::ImageLayout::eTransferDstOptimal
        ) {
      dstAccessFlags = vk::AccessFlagBits::eTransferWrite;
      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
      // Transfer Dest --> Shader Reading
    else if (_imageLayout == vk::ImageLayout::eTransferDstOptimal
             && newLayout == vk::ImageLayout::eReadOnlyOptimal
        ) {
      srcAccessFlags = vk::AccessFlagBits::eTransferWrite;
      dstAccessFlags = vk::AccessFlagBits::eShaderRead;

      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
      // Shader Reading --> Transfer Dest
    else if (_imageLayout == vk::ImageLayout::eReadOnlyOptimal
             && newLayout == vk::ImageLayout::eTransferDstOptimal
        ) {
      srcAccessFlags = vk::AccessFlagBits::eNone;
      dstAccessFlags = vk::AccessFlagBits::eTransferWrite;

      srcPipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
      dstPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
      // Unknown Op.
    else {
      throw std::invalid_argument("unsupported layout transition!");
    }


    vk::ImageMemoryBarrier barrier{
        srcAccessFlags, dstAccessFlags, _imageLayout, newLayout, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
        _image.value(), {
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        }
    };

    _imageLayout = newLayout;

    cmdBuffer.pipelineBarrier(
        srcPipelineStageFlags, dstPipelineStageFlags, vk::DependencyFlagBits::eByRegion, {}, {}, barrier
    );
  }

  void copyBufferToImage(const vk::CommandBuffer &cmdBuffer, const VBuffer<uint8_t> &buffer) const {
    vk::BufferImageCopy bufferImageCopy{
        0, 0, 0, {
            vk::ImageAspectFlagBits::eColor, 0, 0, 1
        }, {0, 0, 0}, {_width, _height, 1}
    };


    cmdBuffer.copyBufferToImage(
        buffer.buffer(), _image.value(), vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
  }

  ///// GETTERS & SETTERS ////////////////////////////////////////////////////
  [[nodiscard]] vk::Sampler sampler() const {
    return _imageSampler.value();
  }

  [[nodiscard]] vk::ImageView imageView() const {
    return _imageView.value();
  }

  [[nodiscard]] vk::ImageLayout imageLayout() const {
    return _imageLayout;
  }

  [[nodiscard]] vk::Format format() const {
    return _imageFormat;
  }

  ///// DESTRUCTOR ///////////////////////////////////////////////////////////
  ~VImage() {
    std::cout << "TMP DEBUG! VImage Destructor Called... ";
    if (_device != nullptr) {
      std::cout << "DESTROYING!" << std::endl;

      if (_imageSampler.has_value()) {
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
};


#endif //FOXTALK_VIMAGE_H
