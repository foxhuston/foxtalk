//
// Created by fox on 10/3/24.
//

#ifndef FOXTALK_VBUFFER_H
#define FOXTALK_VBUFFER_H

#include <vulkan/vulkan.hpp>
#include "VulkUtils.h"

template<typename T>
struct VBuffer {
private:
  const vk::Device *_device;
  vk::BufferCreateInfo _bufferInfo;
  std::optional<vk::Buffer> _buffer;
  std::optional<vk::DeviceMemory> _bufferMemory;
  std::optional<void *> _mappedMemory;

public:
  ///// MAIN CONSTRUCTOR /////////////////////////////////////////////////////
  VBuffer(const vk::PhysicalDevice &physicalDevice, const vk::Device &device,
          size_t count, vk::BufferUsageFlags bufferUsageFlags,
          vk::MemoryPropertyFlags memoryPropertyFlags) {
    _bufferInfo = {{}, sizeof(T) * count, bufferUsageFlags};
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
  VBuffer() {}

  VBuffer(const VBuffer &) = delete;

  VBuffer &operator=(const VBuffer &) = delete;

  VBuffer(VBuffer &&other) noexcept
      : _device{other._device}, _buffer{std::move(other._buffer)}, _bufferInfo{std::move(other._bufferInfo)},
        _bufferMemory{std::move(other._bufferMemory)}, _mappedMemory{std::move(other._mappedMemory)} {
    other._device = nullptr;
    other._buffer = std::nullopt;
    other._bufferMemory = std::nullopt;
    other._mappedMemory = std::nullopt;
  }

  VBuffer &operator=(VBuffer &&other) noexcept {
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
  }

  ///// METHODS //////////////////////////////////////////////////////////////
  void *mapBufferMemory() {
    if (_mappedMemory.has_value()) {
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
  [[nodiscard]] vk::Buffer buffer() const {
    return _buffer.value();
  }

  ///// DESTRUCTOR ///////////////////////////////////////////////////////////
  ~VBuffer() {
    if (_buffer.has_value()) {
      _device->destroyBuffer(_buffer.value());
    }

    if (_bufferMemory.has_value()) {
      if (_mappedMemory.has_value()) {
        _device->unmapMemory(_bufferMemory.value());
      }
      _device->freeMemory(_bufferMemory.value());
    }

  }
};

#endif //FOXTALK_VBUFFER_H
