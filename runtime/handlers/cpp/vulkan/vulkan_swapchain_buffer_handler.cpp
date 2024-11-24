// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct FoxtalkVkBufferPtrs {
  VkImageView view{};
  VkDeviceMemory mem{};
  VkImage image{};
  VkFramebuffer framebuffer{};
  VkFence fence{};
  VkCommandBuffer cmd_buffer{};
};

class VulkanSwapchainBufferHandler : public Handler {

public:
protected:
  std::vector<FoxtalkVkBufferPtrs> ptrs{};
  void handle(const std::vector<Tuple> &queryResults) override {
    debug << "got here with size " << queryResults.size() << this;
    if (queryResults.size() != 5) {
      return;
    }

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      log_error("Query results did not include the vulkan logical device");
      return;
    }

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto queue_family_index = logical_device_tuple->at<uint64_t>(6).value();

    auto swapchain_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan swapchain";
        });

    log_debug("found the swapchain tuple...");

    if (swapchain_tuple == queryResults.end()) {
      log_error("Query results did not include a vulkan swapchain");
      return;
    }

    auto swapchain =
        static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());

    auto pixel_format =
        static_cast<VkFormat>(swapchain_tuple->at<uint64_t>(6).value());

    auto render_pass_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan render pass";
        });

    if (render_pass_tuple == queryResults.end()) {
      log_error("Query results did not include the vulkan render pass");
      return;
    }

    auto render_pass =
        static_cast<VkRenderPass>(render_pass_tuple->at<void *>(0).value());

    auto command_pool_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan command pool";
        });

    if (command_pool_tuple == queryResults.end()) {
      log_error("Query results did not include the vulkan command pool");
      return;
    }

    auto command_pool =
        static_cast<VkCommandPool>(command_pool_tuple->at<void *>(0).value());

    log_debug("found the swapchain!");
    std::cout << "";
    int ptrs_index = 0;
    auto image_symbol_idx =
        swapchain_tuple->index_of(std::string("with images"));
    if (image_symbol_idx == std::nullopt) {
      log_error(" Image pointers not found in "
                "swapchain tuple!");
      return;
    }

    for (size_t i = image_symbol_idx.value() + 1; i < swapchain_tuple->size();
         i++) {
      ptrs.push_back(FoxtalkVkBufferPtrs{});
      auto img = static_cast<VkImage>(swapchain_tuple->at<void *>(i).value());
      debug << "Found image at ptr " <<  img << this;
      ptrs[ptrs_index].image = img;

      VkImageViewCreateInfo iv_create_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image = img,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = pixel_format,
          .components =
              {
                  .r = VK_COMPONENT_SWIZZLE_R,
                  .g = VK_COMPONENT_SWIZZLE_G,
                  .b = VK_COMPONENT_SWIZZLE_B,
                  .a = VK_COMPONENT_SWIZZLE_A,
              },
          .subresourceRange =
              {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1, // TODO: This... hmm, feels like something we
                                   // need to query for
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
      };
      VkResult r;
      r = vkCreateImageView(logical_device, &iv_create_info, nullptr,
                            &ptrs[ptrs_index].view);
      if (r != VK_SUCCESS) {
        log_error("Could not create image view");
        return;
      }
      VkFramebufferCreateInfo fb_create_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass = render_pass,
          .attachmentCount = 1,
          .pAttachments = &ptrs[ptrs_index].view,
          .width = 500,
          .height = 500,
          .layers = 1};
      r = vkCreateFramebuffer(logical_device, &fb_create_info, nullptr,
                              &ptrs[ptrs_index].framebuffer);
      if (r != VK_SUCCESS) {
        log_error("Could not create frame buffer");
        return;
      }

      VkFenceCreateInfo fence_create_info{
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .flags = VK_FENCE_CREATE_SIGNALED_BIT};
      r = vkCreateFence(logical_device, &fence_create_info, nullptr,
                        &ptrs[ptrs_index].fence);
      if (r != VK_SUCCESS) {
        log_error("[Vulkan Swapchain Buffer Handler] Could not create fence");
        return;
      }

      VkCommandBufferAllocateInfo cb_create_info{

          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = command_pool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = 1,
      };
      r = vkAllocateCommandBuffers(logical_device, &cb_create_info,
                                   &ptrs[ptrs_index].cmd_buffer);
      if (r != VK_SUCCESS) {
        log_error("Could not allocate command buffer");
        return;
      }

      claim({{
          {ptrs[ptrs_index].image},
          {"is a vulkan image"},
          {"for device"},
          {logical_device},
          {"with image view"},
          {ptrs[ptrs_index].view},
          {"with frame buffer"},
          {ptrs[ptrs_index].framebuffer},
          {"with fence"},
          {ptrs[ptrs_index].fence},
          {"with command buffer"},
          {ptrs[ptrs_index].cmd_buffer},
          // {"with memory at"},
      }});

      ptrs_index++;
    }
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan swapchain"},
            {"for device"},
            TupleNoun::query(),
            {"with pixel format value"},
            TupleNoun::prefix(),
            {"with images"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});

    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
            {"with graphics queue"},
            TupleNoun::query(),
            {"with queue family index"},
            TupleNoun::query(),
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan render pass"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan command pool"},
            TupleNoun::prefix()}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("is a vulkan image"))) {

      auto img_view = static_cast<VkImageView>(t.at<void *>(5).value());
      auto fb = static_cast<VkFramebuffer>(t.at<void *>(7).value());
      auto fence = static_cast<VkFence>(t.at<void *>(9).value());
      auto cmd_buffer = static_cast<VkCommandBuffer>(t.at<void *>(11).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(3).value());

      vkDestroyFramebuffer(logical_device, fb, nullptr);
      vkDestroyFence(logical_device, fence, nullptr);
      vkDestroyImageView(logical_device, img_view, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainBufferHandler);