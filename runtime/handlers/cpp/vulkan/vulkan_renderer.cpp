// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <ctime>
#include <foxtalk_handler.hpp>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Holy shit this thing is bigger than it needs to be right???
class VulkanRenderingHandler : public Handler {
  int current_frame = 0;

public:
  bool poll() override {
    // std::cout << "in poll" << std::endl;
    if (logical_device == nullptr || swapchain == nullptr ||
        render_pass == nullptr || graphics_pipeline == nullptr) {
      return false;
    }

    vk_fence_status = vkGetFenceStatus(logical_device, fences[current_frame]);
    if (vk_fence_status == VK_SUCCESS) {
      // std::cout << "CURRENT FRAME = " << current_frame
      //           << " SHOULD BE AVAILABLE FOR USE.";
      is_called_from_poll = true;
      return true;
    }
    return false;
  }

protected:
  bool is_called_from_poll = false;
  std::vector<VkCommandBuffer> command_buffers{};
  std::vector<VkFence> fences{};
  std::vector<VkSemaphore> img_available_semaphores{};
  std::vector<VkSemaphore> render_complete_semaphores{};
  VkDevice logical_device{};
  VkSwapchainKHR swapchain{};
  VkResult vk_fence_status = VK_ERROR_UNKNOWN;
  VkRenderPass render_pass{};
  VkExtent2D surface_extent{};
  VkPipeline graphics_pipeline{};

  VkQueue graphics_queue{};
  VkQueue present_queue{};

  // TODO TOMORROW (THANKSGIVING):
  //   Pass the image index of the buffers created in the swapchain
  // in the tuple, and use the same index here. I think this will solve the
  // current bug.
  std::vector<VkFramebuffer> buffers{};

  bool is_ready_to_render = false;

  void handle(const std::vector<Tuple> &queryResults) override {
    if (!is_ready_to_render || !is_called_from_poll) {

      is_ready_to_render = false;
      command_buffers.clear();
      fences.clear();
      img_available_semaphores.clear();
      render_complete_semaphores.clear();
      swapchain = nullptr;
      for (const auto &a : queryResults) {
        if (a.matches(2, std::string("vulkan command pool"))) {
          command_buffers.emplace_back(
              static_cast<VkCommandBuffer>(a.at<void *>(7).value()));
        } else if (a.matches(5, std::string("signaling image is available"))) {
          img_available_semaphores.emplace_back(
              static_cast<VkSemaphore>(a.at<void *>(0).value()));
        } else if (a.matches(5, std::string("signaling rendering is done"))) {
          render_complete_semaphores.emplace_back(
              static_cast<VkSemaphore>(a.at<void *>(0).value()));
        } else if (a.matches(5, std::string("signaling drawing is complete"))) {
          fences.emplace_back(static_cast<VkFence>(a.at<void *>(0).value()));
        }
      }
      if (command_buffers.empty()) {
        err << "Command buffers not found in query results" << end;
        return;
      }
      if (render_complete_semaphores.empty()) {
        err << "Render complete semaphores not found in query results" << end;
        return;
      }
      if (img_available_semaphores.empty()) {
        err << "Image available semaphores not found in query results" << end;
        return;
      }
      if (fences.empty()) {
        err << "Fences not found in query results" << end;
        return;
      }

      auto logical_device_tuple = std::find_if(
          queryResults.begin(), queryResults.end(), [](const Tuple &result) {
            return result.at<std::string>(2) == "vulkan logical device";
          });

      if (logical_device_tuple == queryResults.end()) {
        err << "Query results did not include the vulkan logical device" << end;
        return;
      }

      logical_device =
          static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

      graphics_queue =
          static_cast<VkQueue>(logical_device_tuple->at<void *>(4).value());
      // auto graphics_queue_index =
      // logical_device_tuple->at<uint64_t>(6).value();
      present_queue =
          static_cast<VkQueue>(logical_device_tuple->at<void *>(8).value());
      // auto present_queue_index =
      // logical_device_tuple->at<uint64_t>(10).value();

      auto render_pass_tuple = std::find_if(
          queryResults.begin(), queryResults.end(), [](const Tuple &result) {
            return result.at<std::string>(2) == "vulkan render pass";
          });

      if (render_pass_tuple == queryResults.end()) {
        err << "Query results did not include the render pass" << end;
        return;
      }

      render_pass =
          static_cast<VkRenderPass>(render_pass_tuple->at<void *>(0).value());

      auto swapchain_tuple = std::find_if(
          queryResults.begin(), queryResults.end(), [](const Tuple &result) {
            return result.at<std::string>(2) == "vulkan swapchain";
          });

      if (swapchain_tuple == queryResults.end()) {
        err << "Query results did not include a vulkan swapchain" << end;
        return;
      }

      swapchain =
          static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());

      auto swapchain_logical_device =
          static_cast<VkDevice>(swapchain_tuple->at<void *>(10).value());

      if (logical_device != swapchain_logical_device) {
        err << "Logical device in swapchain tuple (" << swapchain_logical_device
            << ") does not match the logical "
               "device in the logical device tuple ("
            << logical_device << "). " << end;
        return;
      }

      auto width = swapchain_tuple->at<uint64_t>(6).value();
      auto height = swapchain_tuple->at<uint64_t>(8).value();

      surface_extent = VkExtent2D{
          .width = (uint32_t)width,
          .height = (uint32_t)height,
      };

      auto swapchain_image_count = swapchain_tuple->at<uint64_t>(14).value();
      buffers.resize(swapchain_image_count);

      auto pipeline_tuple = std::find_if(
          queryResults.begin(), queryResults.end(), [](const Tuple &result) {
            return result.at<std::string>(2) == "vulkan graphics pipeline";
          });

      if (pipeline_tuple == queryResults.end()) {
        err << "Query results did not include a graphics pipeline" << end;
        return;
      }

      graphics_pipeline =
          static_cast<VkPipeline>(pipeline_tuple->at<void *>(0).value());

      int found_images = 0;
      for (auto &t : queryResults) {
        if (!t.matches(1, std::string("is a vulkan image"))) {
          continue;
        }
        auto img_index = t.at<uint64_t>(5).value();
        auto frame_buffer = static_cast<VkFramebuffer>(t.at<void *>(9).value());
        auto b_swapchain =
            static_cast<VkSwapchainKHR>(t.at<void *>(11).value());

        if (b_swapchain != swapchain) {
          err << "Buffer swapchain does not match the current swapchain" << end;
          is_ready_to_render = false;
          return;
        }
        found_images++;
        buffers[img_index] = frame_buffer;
        // }
        // buffers.emplace_back(
        //     static_cast<VkFramebuffer>(t.at<void *>(7).value()));
      }
      if (found_images != swapchain_image_count) {
        err << "Only " << found_images << " found in the tuple db... expected" << swapchain_image_count << end;
        return;
      }
      debug << "Got all tuples with " << buffers.size() << " frame buffers"
            << end;
      is_ready_to_render = true;
    }

    if (vk_fence_status == VK_SUCCESS && is_ready_to_render &&
        is_called_from_poll) {
      debug << "READY TO GET IMAGES!!" << end;
    } else {
      debug << "Not ready yet : " << vk_fence_status << end;
      return;
    }
    debug << "Starting drawing for frame " << current_frame << end;

    is_called_from_poll = false;

    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(logical_device, swapchain, 1,
                                        img_available_semaphores[current_frame],
                                        VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      err << "vkAcquireNextImageKHR returend a vkresult that wasn't expected: "
          << result << end;

      is_ready_to_render = false;
      return;
    }
    debug << "About to reset fences! Should have been called from poll... "
             "cur frame: "
          << current_frame << "dims: " << surface_extent.width << "x"
          << surface_extent.height << end;
    vkResetFences(logical_device, 1, &fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame],
                         /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(command_buffers[current_frame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {img_available_semaphores[current_frame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[current_frame];

    VkSemaphore signalSemaphores[] = {
        render_complete_semaphores[current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphics_queue, 1, &submitInfo, fences[current_frame]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    debug << "Image index to render: " << imageIndex << end;

    vkQueuePresentKHR(present_queue, &presentInfo);
    current_frame = (current_frame + 1) % fences.size();
  }

  void init() override {

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan semaphore"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan fence"},
            TupleNoun::prefix()}});
    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan render pass"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan swapchain"},
            {"at version"},
            TupleNoun::query(),
            {"for surface of width"},
            TupleNoun::query(),
            {"and height"},
            TupleNoun::query(),
            {"for device"},
            TupleNoun::query(),
            {"with pixel format value"},
            TupleNoun::query(),
            {"with"},
            TupleNoun::query(),
            {"images"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan command pool"},
            TupleNoun::prefix()}});

    claim({{{"vulkan"},
            {"should have"},
            TupleNoun::query(),
            {"frames in flight"}}});

    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan graphics pipeline"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a vulkan image"},
            {"for device"},
            TupleNoun::query(),
            {"at index"},
            TupleNoun::query(),
            {"with image view"},
            TupleNoun::query(),
            {"with frame buffer"},
            TupleNoun::prefix()}});
  }

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = buffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = surface_extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)surface_extent.width;
    viewport.height = (float)surface_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = surface_extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      err << "failed to record command buffer!" << end;
    } else {
      debug << "Recorded command buffer for framebuffer index " << imageIndex
            << end;
    }
  }

  void free_tuple(const Tuple &t) override {}
};

FOXTALK_FFI_HANDLER_REG(VulkanRenderingHandler);