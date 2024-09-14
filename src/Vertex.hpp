//
// Created by fox on 9/13/24.
//

#ifndef FOXTALK_VERTEX_H
#define FOXTALK_VERTEX_H

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions() {
    vk::VertexInputBindingDescription bindingDescription {
      0, sizeof(Vertex), vk::VertexInputRate::eVertex
    };
    return { bindingDescription };
  }

  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
    vk::VertexInputAttributeDescription one {
      0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)
    };

    vk::VertexInputAttributeDescription two {
      1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)
    };

    return { one, two };
  }
};



#endif // FOXTALK_VERTEX_H
