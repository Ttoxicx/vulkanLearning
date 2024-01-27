#pragma once

#include "../base.h"
#include "device.h"
#include "shader.h"
#include "render_pass.h"

namespace FF::Wrapper {
	class Pipeline {
	public:
		using Ptr = std::shared_ptr<Pipeline>;
		static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass) {
			return std::make_shared<Pipeline>(device, renderPass);
		}

		Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

		~Pipeline();

		void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

		void setViewports(const std::vector<VkViewport>& viewports) { _viewports = viewports; }

		void setScissors(const std::vector<VkRect2D>& scissors) { _scissors = scissors; }

		void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment) {
			_blendAttachmentStates.push_back(blendAttachment);
		}

		void build();

	public:

		[[nodiscard]] VkPipeline getPipeline() const { return _pipeline; }

		[[nodiscard]] VkPipelineLayout getPipelineLayout() const { return _layout; }

	public:
		VkPipelineVertexInputStateCreateInfo mVertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};
		VkPipelineViewportStateCreateInfo mViewportState{};
		VkPipelineRasterizationStateCreateInfo mRasterState{};
		VkPipelineMultisampleStateCreateInfo mSampleState{};
		VkPipelineColorBlendStateCreateInfo mBlendState{};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
		VkPipelineLayoutCreateInfo mLayoutCreateInfo{};

	private:
		VkPipeline _pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout _layout{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
		RenderPass::Ptr _renderPass{ nullptr };
		std::vector<Shader::Ptr> _shaders{};
		std::vector<VkViewport> _viewports{};
		std::vector<VkRect2D> _scissors{};
		std::vector<VkPipelineColorBlendAttachmentState> _blendAttachmentStates{};
	};
}