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
    std::cout << "got here with size " << queryResults.size() << std::endl;
    if (queryResults.size() != 4) {
      return;
    }

    auto logical_device_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan logical device";
        });

    if (logical_device_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan logical device"
                << std::endl;
      return;
    }

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto queue_family_index = logical_device_tuple->at<uint64_t>(6).value();

    auto swapchain_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan swapchain";
        });

    if (swapchain_tuple == queryResults.end()) {
      std::cerr << "Query results did not include a vulkan swapchain"
                << std::endl;
      return;
    }

    auto swapchain =
        static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());

    auto pixel_format =
        static_cast<VkFormat>(swapchain_tuple->at<uint64_t>(4).value());

    auto render_pass_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan render pass";
        });

    if (render_pass_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan render pass"
                << std::endl;
      return;
    }

    auto render_pass =
        static_cast<VkRenderPass>(render_pass_tuple->at<void *>(0).value());

    auto command_pool_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan command pool";
        });

    if (command_pool_tuple == queryResults.end()) {
      std::cerr << "Query results did not include the vulkan command pool"
                << std::endl;
      return;
    }

    auto command_pool =
        static_cast<VkCommandPool>(command_pool_tuple->at<void *>(0).value());

    std::cout << "[SwapchainBufferHandler] found the swapchain!" << std::endl;

    int ptrs_index = 0;
    auto image_symbol_idx = swapchain_tuple->index_of(std::string("with images"));
    if (image_symbol_idx == std::nullopt) {
      std::cerr << "[Vulkan Swapchain Buffer Handler] Image pointers not found in swapchain tuple!" << std::endl;
      return;
    }
    for (size_t i = image_symbol_idx.value() + 1; i < swapchain_tuple->size(); i++) {
      ptrs.push_back(FoxtalkVkBufferPtrs{});
      auto img = static_cast<VkImage>(swapchain_tuple->at<void *>(i).value());
      std::cout << "Found image: " << img << std::endl;
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
        std::cerr
            << "[Vulkan Swapchain Buffer Handler] Could not create image view"
            << std::endl;
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
        std::cerr
            << "[Vulkan Swapchain Buffer Handler] Could not create frame buffer"
            << std::endl;
        return;
      }

      VkFenceCreateInfo fence_create_info{
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .flags = VK_FENCE_CREATE_SIGNALED_BIT};
      r = vkCreateFence(logical_device, &fence_create_info, nullptr,
                        &ptrs[ptrs_index].fence);
      if (r != VK_SUCCESS) {
        std::cerr << "[Vulkan Swapchain Buffer Handler] Could not create fence"
                  << std::endl;
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
        std::cerr << "[Vulkan Swapchain Buffer Handler] Could not allocate "
                     "command buffer"
                  << std::endl;
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

    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        {"with graphics queue"},
        TupleNoun::query(),
        {"with queue family index"},
        TupleNoun::query(),
        TupleNoun::prefix()
    }});

    claim({{TupleNoun::query(), {"is a"}, {"vulkan render pass"}, TupleNoun::prefix()}});

    claim({{TupleNoun::query(), {"is a"}, {"vulkan command pool"}, TupleNoun::prefix()}});
  }
  
  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("is a vulkan image"))) {
      
      auto render_pass = static_cast<VkRenderPass>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());

      vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator)
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainBufferHandler);