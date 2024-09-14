//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_GRAPHICS_PIPELINE_H
#define FOXTALK_TEST_GRAPHICS_PIPELINE_H

#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include "shader.hpp"

/**
 * GraphicsPipeline expects `V` to provide:
 *  V::getBindingDescriptions() and
 *  V::getAttributeDescriptions()
 */
template<typename V>
class GraphicsPipeline {
  public:
    GraphicsPipeline() { }

    // No copy...
    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    // Only move.
    GraphicsPipeline(GraphicsPipeline &&other) {
      this->_device           = std::move(other._device);
      this->_renderPass       = std::move(other._renderPass);
      this->_pipelineLayout   = std::move(other._pipelineLayout);
      this->_graphicsPipeline = std::move(other._graphicsPipeline);
      this->_vertexShader     = std::move(other._vertexShader);
      this->_fragmentShader   = std::move(other._fragmentShader);

      other._device           = std::nullopt;
      other._renderPass       = std::nullopt;
      other._pipelineLayout   = std::nullopt;
      other._graphicsPipeline = std::nullopt;
      other._vertexShader     = std::nullopt;
      other._fragmentShader   = std::nullopt;
    }

    GraphicsPipeline &operator=(GraphicsPipeline &&other) {
      this->_device           = std::move(other._device);
      this->_renderPass       = std::move(other._renderPass);
      this->_pipelineLayout   = std::move(other._pipelineLayout);
      this->_graphicsPipeline = std::move(other._graphicsPipeline);
      this->_vertexShader     = std::move(other._vertexShader);
      this->_fragmentShader   = std::move(other._fragmentShader);

      other._device           = std::nullopt;
      other._renderPass       = std::nullopt;
      other._pipelineLayout   = std::nullopt;
      other._graphicsPipeline = std::nullopt;
      other._vertexShader     = std::nullopt;
      other._fragmentShader   = std::nullopt;

      return *this;
    };

    GraphicsPipeline(
        const vk::Device &device,
        const vk::RenderPass &renderPass,
        Shader &&vertexShader,
        Shader &&fragmentShader)
      : _device { device }
      , _renderPass{renderPass}
      , _vertexShader{std::move(vertexShader)}
      , _fragmentShader{std::move(fragmentShader)}
    {
      ///// GRAPHICS PIPELINE //////////////////////////////////////////////////
      auto shaderStages = {
        _vertexShader.value().shaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex),
        _fragmentShader.value().shaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment)
      };

      ///// GEOMETRY ///////////////////////////////////////////////////////////
      auto vertexBindingDescriptions = V::getBindingDescriptions();
      auto vertexAttributeDescriptions = V::getAttributeDescriptions();
      vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
        {}, vertexBindingDescriptions, vertexAttributeDescriptions
      };

      ///// INPUT ASSEMBLY /////////////////////////////////////////////////////
      vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {
        {}, vk::PrimitiveTopology::eTriangleList, vk::False
      };
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
        , vk::FrontFace::eCounterClockwise
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
      _pipelineLayout = _device.value().createPipelineLayout(pipelineLayoutCreateInfo); 

      ///// PIPELINE ///////////////////////////////////////////////////////////
      vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        {}
        , shaderStages
        , &vertexInputInfo
        , &pipelineInputAssemblyStateCreateInfo
        , {}
        , &viewportStateCreateInfo
        , &rasterizerCreateInfo
        , &multisampleStateCreateInfo
        , {}
        , &colorBlendingCreateInfo
        , &dynamicStateCreateInfo
        , _pipelineLayout.value()
        , _renderPass.value()
        , 0
      };

      // TODO: Why does this one need a `.value` when it seemed like the rest of the
      //       `VKResult`s didn't?
      _graphicsPipeline = _device.value().createGraphicsPipeline(nullptr, pipelineCreateInfo).value;
    }

    ~GraphicsPipeline() {
      if(_device.has_value()) {
        if(_graphicsPipeline.has_value()) {
          _device.value().destroyPipeline(_graphicsPipeline.value());
        }

        if(_pipelineLayout.has_value()) {
          _device.value().destroyPipelineLayout(_pipelineLayout.value());
        }
      }
    }

    ///// GETTERS & SETTERS //////////////////////////////////////////////////
    // TODO: What is this copying??
    vk::Pipeline pipeline() const {
      if(_graphicsPipeline.has_value()) {
        return _graphicsPipeline.value();
      }

      throw new std::runtime_error("Tried to get pipeline from a moved object!");
    }


  private:
    std::optional<vk::Device> _device;
    std::optional<vk::RenderPass> _renderPass;
    std::optional<vk::PipelineLayout> _pipelineLayout;
    std::optional<vk::Pipeline> _graphicsPipeline;
    std::optional<Shader> _vertexShader;
    std::optional<Shader> _fragmentShader;
};

#endif // FOXTALK_TEST_GRAPHICS_PIPELINE_H
