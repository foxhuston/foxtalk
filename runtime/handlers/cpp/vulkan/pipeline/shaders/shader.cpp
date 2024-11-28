// pkg-config vulkan

#include <cstdlib>
#include <cstring>
#include <foxtalk_handler.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        return bindingDescription;
    }
};
const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
 
class ShaderLoadingHandler : public Handler {
protected:
  // pulled directly from vulkan-tutorial
  std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
  }

  std::optional<VkShaderModule>
  createShaderModule(VkDevice device, const std::string &file_name,
                     const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS) {
      err << "Could not create shader module from  " << file_name << end;
      return std::nullopt;
    }
    return shaderModule;
  }
  void handle(const std::vector<Tuple> &queryResults) override {

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      err << "Query results did not include the vulkan logical device" << end;
      return;
    }

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto frag_shader_path = std::format("{}{}", std::getenv("SO_PATH"),
                                        "/vulkan/pipeline/shaders/shader.frag.spv");

    auto vert_shader_path = std::format("{}{}", std::getenv("SO_PATH"),
                                        "/vulkan/pipeline/shaders/shader.vert.spv");

    auto vertShaderCode = readFile(vert_shader_path);
    auto fragShaderCode = readFile(frag_shader_path);

    auto frag_shader_module = createShaderModule(logical_device, frag_shader_path, fragShaderCode);
    auto vert_shader_module = createShaderModule(logical_device, vert_shader_path, vertShaderCode);
    if (!frag_shader_module || !vert_shader_module) {
      return;
    }

    claim({{
      {frag_shader_module.value()},
      {"is a"},
      {"frag"},
      {"shader module"},
      {"for device"},
      {logical_device}
    }});

    claim({{
      {vert_shader_module.value()},
      {"is a"},
      {"vert"},
      {"shader module"},
      {"for device"},
      {logical_device}
    }});

  }

  void free_tuple(const Tuple& t) override {
    if (t.matches(3, std::string("shader module"))) {

      auto shader = static_cast<VkShaderModule>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(5).value());
      debug << "Freeing shader module " << t << end;
      // vkDestroyShaderModule(logical_device, shader, nullptr);
    }
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});

    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        TupleNoun::prefix(),
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(ShaderLoadingHandler);