//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_FOXTALK_H
#define FOXTALK_FOXTALK_H

#include "boot/Core.hpp"
#include "shader.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

class Foxtalk {
  public:
    Foxtalk(Core *__core) : _core { __core },
      _vertexShader { __core, "src/shaders/simple.vert.bin" },
      _fragmentShader { __core, "src/shaders/simple.frag.bin" }
      {
      ///// GRAPHICS PIPELINE //////////////////////////////////////////////////

      auto shaderStages = {
        _vertexShader.shaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex),
        _fragmentShader.shaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment)
      };

      ///// GEOMETRY ///////////////////////////////////////////////////////////
      vk::PipelineVertexInputStateCreateInfo vertexInputInfo {};

      ///// INPUT ASSEMBLY /////////////////////////////////////////////////////
      vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {
        {}, vk::PrimitiveTopology::eTriangleList, vk::False
      };

      /* ///// VIEWPORT /////////////////////////////////////////////////////////// */
      /* vk::Viewport viewport { */
      /*   0.0f, 0.0f, */
      /*   static_cast<float>(core().swapchainExtent().width), */
      /*   static_cast<float>(core().swapchainExtent().height), */
      /*   0.0f, 1.0f */
      /* }; */

      /* ///// SCISSOR //////////////////////////////////////////////////////////// */
      /* vk::Rect2D scissor { */
      /*   { 0, 0 }, */
      /*   core().swapchainExtent() */
      /* }; */

      ///// VIEWPORT STATE /////////////////////////////////////////////////////
      std::vector<vk::DynamicState> dynamicStates {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
      };

      vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo { {}, dynamicStates };

      vk::PipelineViewportStateCreateInfo viewportStateCreateInfo {
        {}, 1, {}, 1, {} // using dynamic state here.
      };

      ///// RASTERIZER /////////////////////////////////////////////////////////
      vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo {
        {}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack
        , vk::FrontFace::eClockwise // TODO: Change this nonsense?
        , vk::False, {}, {}, {}
        , 1.0f
      };

      ///// MULTISAMPLING //////////////////////////////////////////////////////
      vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo { };

      ///// DEPTH & STENCIL TESTING ////////////////////////////////////////////
      /// TODO!

      ///// COLOR BLENDING /////////////////////////////////////////////////////
      vk::PipelineColorBlendAttachmentState colorBlendAttachmentState { };
      colorBlendAttachmentState.setColorWriteMask(
            vk::ColorComponentFlagBits::eR
          | vk::ColorComponentFlagBits::eG
          | vk::ColorComponentFlagBits::eB
          | vk::ColorComponentFlagBits::eA
      );
      colorBlendAttachmentState.setBlendEnable(vk::True);
      colorBlendAttachmentState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
      colorBlendAttachmentState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
      colorBlendAttachmentState.setColorBlendOp(vk::BlendOp::eAdd);
      colorBlendAttachmentState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
      colorBlendAttachmentState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
      colorBlendAttachmentState.setAlphaBlendOp(vk::BlendOp::eAdd);

      vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo {
        {}, vk::False
        , vk::LogicOp::eCopy
        , { colorBlendAttachmentState }
        , { 0.0f, 0.0f, 0.0f, 0.0f }
      };

      ///// PIPELINE LAYOUT ////////////////////////////////////////////////////
      vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo { };
      _pipelineLayout = core().device().createPipelineLayout(pipelineLayoutCreateInfo); 

      ///// PIPELINE ///////////////////////////////////////////////////////////
      vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        {}
        , shaderStages
        , &vertexInputInfo
        , &pipelineInputAssemblyStateCreateInfo
        , {}
        , &viewportStateCreateInfo
        , &rasterizerCreateInfo
        , {}
        , {}
        , &colorBlendingCreateInfo
        , &dynamicStateCreateInfo
        , _pipelineLayout
        , _renderPass
        , 0
      };

      // TODO: Why does this one need a `.value` when it seemed like the rest of the
      //       `VKResult`s didn't?
      _graphicsPipeline = core().device().createGraphicsPipeline(nullptr, pipelineCreateInfo).value;

      ///// COLOR ATTACHMENT DESCRIPTION ///////////////////////////////////////
      std::vector<vk::AttachmentDescription> colorAttachmentDescriptions {
        {
          {}
          , core().swapchainImageFormat()
          , vk::SampleCountFlagBits::e1
          , vk::AttachmentLoadOp::eClear
          , vk::AttachmentStoreOp::eStore
          , vk::AttachmentLoadOp::eDontCare
          , vk::AttachmentStoreOp::eDontCare
          , vk::ImageLayout::eUndefined
          , vk::ImageLayout::ePresentSrcKHR
        }
      };

      ///// RENDER SUBPASSES ///////////////////////////////////////////////////
      std::vector<vk::AttachmentReference> colorAttachmentReferences {
        {
          {}
          , vk::ImageLayout::eColorAttachmentOptimal
        }
      };

      std::vector<vk::SubpassDescription> subpassDescriptions {
        {
          {}
          , vk::PipelineBindPoint::eGraphics
          , {}
          , colorAttachmentReferences
        }
      };

      ///// RENDER PASS ////////////////////////////////////////////////////////
      vk::RenderPassCreateInfo renderPassCreateInfo {
        {}
        , colorAttachmentDescriptions
        , subpassDescriptions
      };

      _renderPass = core().device().createRenderPass(renderPassCreateInfo);
    }

    Core& core() const {
      return *_core;
    }

    ~Foxtalk() {
      core().device().destroyPipeline(_graphicsPipeline);
      core().device().destroyPipelineLayout(_pipelineLayout);
      core().device().destroyRenderPass(_renderPass);
    }

  private:
    Core *_core;
    Shader _vertexShader;
    Shader _fragmentShader;


    vk::RenderPass _renderPass;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;

};

#endif // FOXTALK_FOXTALK_H
