//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_SHADER_H
#define FOXTALK_SHADER_H

#include <fstream>
#include <vulkan/vulkan.hpp>

#include "Core.hpp"

class Shader {
  public:
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Core *__core, const std::string &fileName) : _core{__core} {
      auto shader_code = readFile(fileName);
      vk::ShaderModuleCreateInfo createInfo {};

      // Hmm.....
      createInfo.setPCode(reinterpret_cast<const uint32_t*>(shader_code.data()));
      createInfo.setCodeSize(shader_code.size());

      _shaderModule = core().device().createShaderModule(createInfo);
    }

    Core& core() const {
      return *_core;
    }

    const vk::ShaderModule shaderModule() const {
      return _shaderModule;
    }

    const vk::PipelineShaderStageCreateInfo shaderStageCreateInfo(vk::ShaderStageFlagBits shaderStage) const {
      return {
        {}, shaderStage, _shaderModule, "main"
      };
    }

    ~Shader() {
      core().device().destroyShaderModule(_shaderModule);
    }

  private:
    Core *_core;
    vk::ShaderModule _shaderModule;

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
