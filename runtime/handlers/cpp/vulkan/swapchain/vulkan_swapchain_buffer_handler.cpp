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

      auto surface_extent_tuple = std::find_if(
          queryResults.begin(), queryResults.end(), [](const Tuple &result) {
            return result.at<std::string>(0) == "available surface has width";
          });

      if (surface_extent_tuple == queryResults.end()) {
        err << "Query results did not include the available surface extent" << end;
        return;
      }

      auto width = surface_extent_tuple->at<uint64_t>(1).value();
      auto height = surface_extent_tuple->at<uint64_t>(3).value();

    auto swapchain_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vulkan swapchain";
        });

    debug << "found the swapchain tuple..." << end;

    if (swapchain_tuple == queryResults.end()) {
      err << "Query results did not include a vulkan swapchain" << end;
      return;
    }

    auto swapchain =
        static_cast<VkSwapchainKHR>(swapchain_tuple->at<void *>(0).value());

    auto pixel_format =
        static_cast<VkFormat>(swapchain_tuple->at<uint64_t>(8).value());

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
    int ptrs_index = 0;
    auto image_symbol_idx =
        swapchain_tuple->index_of(std::string("with images"));
    if (image_symbol_idx == std::nullopt) {
      err << " Image pointers not found in swapchain tuple!" << end;
      return;
    }
    debug << "Need to create images: " << image_symbol_idx.value() + 1 << " through " <<  swapchain_tuple->size() << end;
    for (size_t i = image_symbol_idx.value() + 1; i < swapchain_tuple->size();
         i++) {
      ptrs.push_back(FoxtalkVkBufferPtrs{});
      auto img = static_cast<VkImage>(swapchain_tuple->at<void *>(i).value());
      debug << "Found image at ptr " <<  img << end;
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
        err << "Could not create image view" << end;
        return;
      }
      VkFramebufferCreateInfo fb_create_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass = render_pass,
          .attachmentCount = 1,
          .pAttachments = &ptrs[ptrs_index].view,
          .width = (uint32_t)width,
          .height = (uint32_t)height,
          .layers = 1};
      r = vkCreateFramebuffer(logical_device, &fb_create_info, nullptr,
                              &ptrs[ptrs_index].framebuffer);
      if (r != VK_SUCCESS) {
        err << "Could not create frame buffer" << end;
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
          {"using swapchain"},
          {swapchain},
          // {"with memory at"},
      }});

      ptrs_index++;
    }
  }

  void init() override {
    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan swapchain"},
            {"at version"},
            TupleNoun::query(),
            {"for device"},
            TupleNoun::query(),
            {"with pixel format value"},
            TupleNoun::prefix(),
            {"with images"},
            TupleNoun::prefix()}});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});

      claim({{
          {"available surface has width"},
          TupleNoun::query(),
          {"and height"},
          TupleNoun::query(),
      }});
    claim({{TupleNoun::query(),
            {"is the"},
            {"vulkan logical device"},
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
      auto logical_device = static_cast<VkDevice>(t.at<void *>(3).value());

      vkDestroyFramebuffer(logical_device, fb, nullptr);
      vkDestroyImageView(logical_device, img_view, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanSwapchainBufferHandler);