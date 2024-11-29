// pkg-config vulkan

#include "foxtalk_tuple.h"
#include <cstdlib>
#include <cstring>
#include <foxtalk_handler.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// This has a BUNCH of setup in it... we will need to break this out later when
// we want more fine-grained control over it. For now, it all just gets set up
// in here

class VulkanPipelineHandler : public Handler {
protected:
    VkPipelineLayout pipelineLayout{};
  VkPipeline graphicsPipeline{};

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

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};

    for (const auto& q : queryResults) {
      if (!q.matches(3, std::string("shader module"))) {
        continue;
      }
      debug << "Found shader module: " << q << end;

      auto shader = static_cast<VkShaderModule>(q.at<void *>(0).value());
      auto shader_type = q.at<std::string>(2).value();

      VkPipelineShaderStageCreateInfo shader_stage_info{};
      shader_stage_info.sType =
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      VkShaderStageFlagBits stage; 
      if (shader_type == "vert") {
        stage = VK_SHADER_STAGE_VERTEX_BIT;
      } 
      if (shader_type == "frag") {
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      }
      shader_stage_info.stage = stage;
      shader_stage_info.module = shader;
      shader_stage_info.pName = "main";
      shader_stages.emplace_back(shader_stage_info);
    }

    // note, this is just for the triangle.
    if (shader_stages.size() != 2) {
      err << "Did not find two shader stages. Found " << shader_stages.size() << ".  Stopping here." << end;
      return;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    auto result = vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo,
                                         nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
      err << "failed to create pipeline layout! Result: " << result << end;
      return;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader_stages.data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional


    auto gfxResult = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, nullptr,
                                  &graphicsPipeline);
    if (gfxResult != VK_SUCCESS) {
      err << "failed to create graphics pipeline!" << end;
      return;
    }

    claim({{{graphicsPipeline},
            {"is the"},
            {"vulkan graphics pipeline"},
            {"with pipeline layout"},
            {pipelineLayout},
            {"for device"},
            {logical_device}}});
  }

  void free_tuple(const Tuple &t) override {
    if (t.matches(2, std::string("vulkan graphics pipeline"))) {
      
      auto pipeline = static_cast<VkPipeline>(t.at<void *>(0).value());
       
      auto pipeline_layout = static_cast<VkPipelineLayout>(t.at<void *>(4).value());
      auto logical_device = static_cast<VkDevice>(t.at<void *>(6).value());
      debug << "Freeing pipeline and layout " << t << end;
      vkDestroyPipeline(logical_device, pipeline, nullptr);
      vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    }
  }

  void init() override {
    claim({{
        TupleNoun::query(),
        {"is the"},
        {"vulkan logical device"},
        TupleNoun::prefix(),
    }});

    claim({{TupleNoun::query(),
            {"is a"},
            {"vulkan render pass"},
            TupleNoun::prefix()}});

    claim({{
        TupleNoun::query(),
        {"is a"},
        TupleNoun::query(),
        {"shader module"},
        TupleNoun::prefix(),
    }});
  }
};

FOXTALK_FFI_HANDLER_REG(VulkanPipelineHandler);