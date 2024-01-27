#include "pipeline.h"

namespace FF::Wrapper {
	Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass) {
		_device = device;
		_renderPass = renderPass;
		mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	}

	Pipeline::~Pipeline() {
		if (_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(_device->getDevice(), _layout, nullptr);
		}

		if (_pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(_device->getDevice(), _pipeline, nullptr);
		}
	}

	void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup) {
		_shaders = shaderGroup;
	}

	void Pipeline::build() {

		//����shaders
		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
		for (const auto& shader : _shaders) {
			VkPipelineShaderStageCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderCreateInfo.stage = shader->getShaderStage();
			shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();
			shaderCreateInfo.module = shader->getShaderModule();
			shaderCreateInfos.push_back(shaderCreateInfo);
		}

		//�����ӿڼ���
		mViewportState.viewportCount = static_cast<uint32_t>(_viewports.size());
		mViewportState.pViewports = _viewports.data();
		mViewportState.scissorCount = static_cast<uint32_t>(_scissors.size());
		mViewportState.pScissors = _scissors.data();

		//blending
		mBlendState.attachmentCount = static_cast<uint32_t>(_blendAttachmentStates.size());
		mBlendState.pAttachments = _blendAttachmentStates.data();

		//layout����
		if (_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(_device->getDevice(), _layout, nullptr);
		}
		if (vkCreatePipelineLayout(_device->getDevice(), &mLayoutCreateInfo, nullptr, &_layout) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create pipelien layout");
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
		pipelineCreateInfo.pStages = shaderCreateInfos.data();

		pipelineCreateInfo.pVertexInputState = &mVertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;
		pipelineCreateInfo.pViewportState = &mViewportState;
		pipelineCreateInfo.pRasterizationState = &mRasterState;
		pipelineCreateInfo.pMultisampleState = &mSampleState;
		pipelineCreateInfo.pDepthStencilState = &mDepthStencilState;
		pipelineCreateInfo.pColorBlendState = &mBlendState;
		pipelineCreateInfo.layout = _layout;
		pipelineCreateInfo.renderPass = _renderPass->getRenderPass();
		pipelineCreateInfo.subpass = 0;

		//�Դ��ڵ�pipelineΪ�������д���������죬������Ҫָ��flagsΪVK_PIPELINE_CREATE_DERIVATIVE_BIT
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		//pipelineCreateInfo.flags=VK_PIPELINE_CREATE_DERIVATIVE_BIT
		pipelineCreateInfo.basePipelineIndex = -1;

		if (_pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(_device->getDevice(), _pipeline, nullptr);
		}

		//pipeline cache�����Խ������Ϣ���뻺�棬�ڶ��pipeline����ʹ�ã�Ҳ���Դ浽�ļ�����ͬ�������
		if (vkCreateGraphicsPipelines(_device->getDevice(), VK_NULL_HANDLE, 1,&pipelineCreateInfo, nullptr, &_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create pipeline");
		}
	}
}