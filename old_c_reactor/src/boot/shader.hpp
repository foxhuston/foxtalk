//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_SHADER_H
#define FOXTALK_SHADER_H

#include <iostream>
#include <optional>
#include <fstream>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

class Shader {
  public:
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&other)
      : _device { std::move(other._device) }
      , _shaderModule { std::move(other._shaderModule) }
    {
      other._device = nullptr;
      other._shaderModule = std::nullopt;
    }

    Shader &operator=(Shader &&other) {
      std::cout << "TMP DEBUG! Shader move operator called." << std::endl;
      this->_device = std::move(other._device);
      this->_shaderModule = std::move(other._shaderModule);

      other._device = nullptr;
      other._shaderModule = std::nullopt;

      return *this;
    }

    Shader() {}

    Shader(const vk::Device *__device, const std::string &fileName) : _device { __device } {
      auto shader_code = readFile(fileName);
      vk::ShaderModuleCreateInfo createInfo {};

      // Hmm.....
      createInfo.setPCode(reinterpret_cast<const uint32_t*>(shader_code.data()));
      createInfo.setCodeSize(shader_code.size());

      _shaderModule = __device->createShaderModule(createInfo);
    }

    const vk::ShaderModule shaderModule() const {
      return _shaderModule.value();
    }

    const vk::PipelineShaderStageCreateInfo shaderStageCreateInfo(vk::ShaderStageFlagBits shaderStage) const {
      return {
        {}, shaderStage, shaderModule(), "main"
      };
    }

    ~Shader() {
      // If we haven't been moved...
      if(_shaderModule.has_value() && _device != nullptr) {
        std::cout << "TMP DEBUG! Shader destructor called, now calling destroyShaderModule " << std::endl;
        _device->destroyShaderModule(_shaderModule.value());
      } else {
        std::cout << "TMP DEBUG! Shader destructor called... I've been moved!" << std::endl;
      }
    }

  private:
    const vk::Device *_device;
    std::optional<vk::ShaderModule> _shaderModule;

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
};

#endif // FOXTALK_SHADER_H
