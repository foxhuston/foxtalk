//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

class Foxtalk {
  public:
    void tick() { }

    void render(const vk::CommandBuffer& cmdBuffer) {
        cmdBuffer.draw(3, 1, 0, 0);
    }
};

#endif // FOXTALK_FOXTALK_H
