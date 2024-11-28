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
};

class VulkanSwapchainBufferHandler : public Handler {

public:
protected:
  std::vector<FoxtalkVkBufferPtrs> ptrs{};
  void handle(const std::vector<Tuple> &queryResults) override {
    auto swapchain_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan swapchain";
        });

    if (swapchain_tuple == queryResults.end()) {
      err << "Query results did not include a vulkan swapchain" << end;
      return;
    }

    auto swapchain =
        static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());

    auto pixel_format =
        static_cast<VkFormat>(swapchain_tuple->at<uint64_t>(12).value());
        

    auto logical_device =
        static_cast<VkDevice>(swapchain_tuple->at<void *>(10).value());


    auto width = swapchain_tuple->at<uint64_t>(6).value();
    auto height = swapchain_tuple->at<uint64_t>(8).value();
    auto render_pass_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan render pass";
        });

    if (render_pass_tuple == queryResults.end()) {
      err << "Query results did not include the vulkan render pass" << end;
      return;
    }

    auto render_pass =
        static_cast<VkRenderPass>(render_pass_tuple->at<void *>(0).value());

    auto command_pool_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan command pool";
        });

    if (command_pool_tuple == queryResults.end()) {
      err << "Query results did not include the vulkan command pool" << end;
      return;
    }

    auto command_pool =
        static_cast<VkCommandPool>(command_pool_tuple->at<void *>(0).value());

    debug << "found the swapchain!" << end;
    auto num_images = swapchain_tuple->at<uint64_t>(14).value();
    // first image cptr
    auto first_image_idx = 16;
    ptrs.resize(num_images);
    for (auto i = 0; i < num_images;
         i++) { // 12, 14... 15, 17... 18, 20... 21, 23
      auto image_index = first_image_idx + (i * 3);
      auto image_index_index = image_index + 2;
      auto img = static_cast<VkImage>(
          swapchain_tuple->at<void *>(image_index).value());
      auto img_index = swapchain_tuple->at<uint64_t>(image_index_index).value();
      debug << "Found image at ptr " << img << end;
      ptrs[img_index].image = img;

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
                            &ptrs[img_index].view);
      if (r != VK_SUCCESS) {
        err << "Could not create image view " << r << end;
        return;
      }
      VkFramebufferCreateInfo fb_create_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass = render_pass,
          .attachmentCount = 1,
          .pAttachments = &ptrs[img_index].view,
          .width = (uint32_t)width,
          .height = (uint32_t)height,
          .layers = 1};
      r = vkCreateFramebuffer(logical_device, &fb_create_info, nullptr,
                              &ptrs[img_index].framebuffer);
      if (r != VK_SUCCESS) {
        err << "Could not create frame buffer: " << r << end;
        return;
      }

      claim({{
          {ptrs[img_index].image},
          {"is a vulkan image"},
          {"for device"},
          {logical_device},
          {"at index"},
          {img_index},
          {"with image view"},
          {ptrs[img_index].view},
          {"with frame buffer"},
          {ptrs[img_index].framebuffer},
          {"using swapchain"},
          {swapchain},
          // {"with memory at"},
      }});
    }
  }

  void init() override {
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

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});

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

      auto img_view = static_cast<VkImageView>(t.at<void *>(7).value());
      auto fb = static_cast<VkFramebuffer>(t.at<void *>(9).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(3).value());
      debug << "Freeing image view and frame buffer " << t << end;
      // vkDestroyFramebuffer(logical_device, fb, nullptr);
      // vkDestroyImageView(logical_device, img_view, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainBufferHandler);