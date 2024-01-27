#include "render_pass.h"

namespace FF::Wrapper {

	SubPass::SubPass() {

	}

	SubPass::~SubPass() {

	}

	void SubPass::addColorAttachmentReference(const VkAttachmentReference& reference) {
		_colorAttachmentReference.push_back(reference);
	}

	void SubPass::addInputAttachmentReference(const VkAttachmentReference& reference) {
		_InputAttachmentReference.push_back(reference);
	}

	void SubPass::setResolvedAttachmentReference(const VkAttachmentReference& reference) {
		_resolveReference = reference;
	}

	void SubPass::setDepthStencilAttachmentReference(const VkAttachmentReference& reference) {
		_depthStencilReference = reference;
	}

	void SubPass::buildSubPassDescription() {
		if (_colorAttachmentReference.empty()) {
			throw std::runtime_error("Error: color attachment group is empty");
		}

		_subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		_subPassDescription.colorAttachmentCount = static_cast<uint32_t>(_colorAttachmentReference.size());
		_subPassDescription.pColorAttachments = _colorAttachmentReference.data();

		_subPassDescription.inputAttachmentCount = static_cast<uint32_t>(_InputAttachmentReference.size());
		_subPassDescription.pInputAttachments = _InputAttachmentReference.data();

		_subPassDescription.pResolveAttachments = &_resolveReference;
		_subPassDescription.pDepthStencilAttachment = _depthStencilReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &_depthStencilReference;

	}

	RenderPass::RenderPass(const Device::Ptr& device) {
		_device = device;
	}

	RenderPass::~RenderPass() {
		if (_renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(_device->getDevice(), _renderPass, nullptr);
		}
	}

	void RenderPass::addSubPass(const SubPass& subpass) {
		_subPasses.push_back(subpass);
	}

	void RenderPass::addDependency(const VkSubpassDependency& dependency) {
		_dependencies.push_back(dependency);
	}

	void RenderPass::addAttachment(const VkAttachmentDescription& description) {
		_attachmentDescriptions.push_back(description);
	}

	void RenderPass::buildPrenderPass() {
		if (_subPasses.empty() || _attachmentDescriptions.empty() || _dependencies.empty()) {
			throw std::runtime_error("Error:no enough elements to build render pass");
		}

		//unwrap
		std::vector<VkSubpassDescription> subPasses{};
		for (int i = 0; i < _subPasses.size(); ++i) {
			subPasses.push_back(_subPasses[i].getSubPassDescription());
		}

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = static_cast<uint32_t>(_attachmentDescriptions.size());
		createInfo.pAttachments = _attachmentDescriptions.data();
		createInfo.dependencyCount = static_cast<uint32_t>(_dependencies.size());
		createInfo.pDependencies = _dependencies.data();
		createInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
		createInfo.pSubpasses = subPasses.data();

		if (vkCreateRenderPass(_device->getDevice(), &createInfo, nullptr, &_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create render pass");
		}
	}
}