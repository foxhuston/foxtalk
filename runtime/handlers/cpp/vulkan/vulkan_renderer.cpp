// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <ctime>
#include <foxtalk_handler.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Holy shit this thing is bigger than it needs to be right???
class VulkanRenderingHandler : public Handler {

public:
  bool poll() override {
    // std::cout << "in poll" << std::endl;
    if (logical_device == nullptr) {
      // std::cout << "early returning because of null logical device" << std::endl;
      return false;
    }
    if (fence == nullptr) {
      // std::cout << "early returning because of null fence" << std::endl;
      return false;
    }
    vk_fence_status = vkGetFenceStatus(logical_device, fence);
    // std::cout << "new fence status = " << vk_fence_status << std::endl;
    return vk_fence_status != VK_NOT_READY;
  }

protected:
  VkCommandBuffer command_buffer{};
  VkFence fence{};
  VkSemaphore img_available_semaphore{};
  VkSemaphore render_complete_semaphore{};
  VkDevice logical_device{};
  VkSwapchainKHR swapchain{};
  VkResult vk_fence_status = VK_ERROR_UNKNOWN;
  VkRenderPass render_pass{};
  VkExtent2D surface_extent{
      .width = 500,
      .height = 500,
  };
  VkPipeline graphics_pipeline{};

  VkQueue graphics_queue{};
  VkQueue present_queue{};
  std::vector<VkFramebuffer> buffers{};

  double get_time_as_double() {
    struct timespec t {};
    clock_gettime(CLOCK_REALTIME, &t);
    return (double)t.tv_sec + ((double)t.tv_nsec / 1E7);  
  }

  void handle(const std::vector<Tuple> &queryResults) override {
    claim({{{"last render attempt at"}, {get_time_as_double()}, {"fence status = "}, {(int64_t)vk_fence_status}}});
    auto command_pool_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan command pool";
        });

    if (command_pool_tuple == queryResults.end()) {
      log_error("Query results did not include the vulkan command pool");
      return;
    }

    command_buffer =
        static_cast<VkCommandBuffer>(command_pool_tuple->at<void *>(6).value());

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      log_error("Query results did not include the vulkan logical device");
      return;
    }

    logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    graphics_queue =
        static_cast<VkQueue>(logical_device_tuple->at<void *>(4).value());
    // auto graphics_queue_index = logical_device_tuple->at<uint64_t>(6).value();
    present_queue =
        static_cast<VkQueue>(logical_device_tuple->at<void *>(8).value());
    // auto present_queue_index = logical_device_tuple->at<uint64_t>(10).value();

    auto fence_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan fence";
        });

    if (fence_tuple == queryResults.end()) {
      log_error("Query results did not include the fence");
      return;
    }

    fence = static_cast<VkFence>(fence_tuple->at<void *>(0).value());

    auto render_pass_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan render pass";
        });

    if (render_pass_tuple == queryResults.end()) {
      log_error("Query results did not include the render pass");
      return;
    }

    render_pass =
        static_cast<VkRenderPass>(render_pass_tuple->at<void *>(0).value());

    auto img_available_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.matches(2, std::string("vulkan semaphore")) &&
                 result.matches(5, std::string("signaling image is available"));
        });

    if (img_available_tuple == queryResults.end()) {
      log_error("Query results did not include the img available semaphore");
      return;
    }

    img_available_semaphore =
        static_cast<VkSemaphore>(img_available_tuple->at<void *>(0).value());

    auto rendering_complete_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.matches(2, std::string("vulkan semaphore")) &&
                 result.matches(5, std::string("signaling rendering is done"));
        });

    if (rendering_complete_tuple == queryResults.end()) {
      log_error("Query results did not include the render done semaphore");
      return;
    }

    render_complete_semaphore = static_cast<VkSemaphore>(
        rendering_complete_tuple->at<void *>(0).value());

    auto swapchain_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan swapchain";
        });

    if (swapchain_tuple == queryResults.end()) {
      log_error("Query results did not include a vulkan swapchain");
      return;
    }

    swapchain =
        static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());
    logical_device =
        static_cast<VkDevice>(swapchain_tuple->at<void *>(4).value());

    auto pipeline_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan graphics pipeline";
        });

    if (pipeline_tuple == queryResults.end()) {
      log_error("Query results did not include a graphics pipeline");
      return;
    }

    graphics_pipeline =
        static_cast<VkPipeline>(pipeline_tuple->at<void *>(0).value());

    for (auto &t : queryResults) {
      if (!t.matches(1, std::string("is a vulkan image"))) {
        continue;
      }
      buffers.emplace_back(static_cast<VkFramebuffer>(t.at<void *>(7).value()));
    }
    if (buffers.size() <= 0) {
      log_error("No frame buffers found in the query results");
      return;
    }
    debug << "Got all tuples with" << buffers.size() << " frame buffers";
    if (vk_fence_status == VK_SUCCESS) {
      log_debug("READY TO GET IMAGES!!");
    } else {
      debug << "Not ready yet : " << vk_fence_status;
      log_existing_debug();
      return;
    }

    vkResetFences(logical_device, 1, &fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX,
                          img_available_semaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(command_buffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(command_buffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {img_available_semaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    VkSemaphore signalSemaphores[] = {render_complete_semaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphics_queue, 1, &submitInfo, fence) != VK_SUCCESS) {
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

    vkQueuePresentKHR(present_queue, &presentInfo);
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
            {"for device"},
            TupleNoun::query(),
            {"with pixel format value"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan command pool"},
            {"for device"},
            TupleNoun::query(),
            {"with command buffer"},
            TupleNoun::query(),
            TupleNoun::prefix()}});

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
      log_error("failed to record command buffer!");
    } else {
      debug << "Recorded command buffer for framebuffer index " << imageIndex;
      log_existing_debug();
    }
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("is a vulkan image"))) {
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanRenderingHandler);