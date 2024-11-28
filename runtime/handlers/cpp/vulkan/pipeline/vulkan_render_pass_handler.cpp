// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdint>
#include <foxtalk_handler.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanRenderPassHandler : public Handler {
public:
protected:
  VkRenderPass render_pass{};
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 3) {
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

    auto logical_device =
        static_cast<VkDevice>(logical_device_tuple->at<void *>(0).value());

    auto surface_tuple = std::find_if(
        queryResults.begin(), queryResults.end(), [](const Tuple &result) {
          return result.at<std::string>(2) == "vk surface khr";
        });

    if (surface_tuple == queryResults.end()) {
      err << "Query results did not include the vk surface khr" << end;
      return;
    }

    auto pixel_format =
        static_cast<VkFormat>(surface_tuple->at<uint64_t>(4).value());
    VkAttachmentDescription attachment_descriptions[]{{
        .format = pixel_format,
        .samples = VK_SAMPLE_COUNT_1_BIT, // why?
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    }};

    VkAttachmentReference color_attachments[]{
        {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkAttachmentReference resolve_attachments[]{
        {.attachment = VK_ATTACHMENT_UNUSED,
         .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkSubpassDescription subpasses[]{{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = color_attachments,
        .pResolveAttachments = resolve_attachments,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    }};

    VkRenderPassCreateInfo render_pass_create_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachment_descriptions,
        .subpassCount = 1,
        .pSubpasses = subpasses,
        .dependencyCount = 0};

    vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr,
                       &render_pass);
    claim({{{render_pass},
            {"is a"},
            {"vulkan render pass"},
            {"for device"},
            {logical_device}}});
  }

  void init() override {
    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        TupleNoun::prefix(),
    }});

    claim({{TupleNoun::query(), {"is the"}, {"vulkan instance"}}});
    claim({{TupleNoun::query(),
            {"is a"},
            {"vk surface khr"},
            {"with surface pixel format value"},
            TupleNoun::query(),
            TupleNoun::prefix()}});
  }
  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan render pass"))) {

      auto render_pass = static_cast<VkRenderPass>(t.at<void *>(0).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(4).value());
      debug << "Freeing render pass " << t << end;
      // vkDestroyRenderPass(logical_device, render_pass, nullptr);
    }
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanRenderPassHandler);