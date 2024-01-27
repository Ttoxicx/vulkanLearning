#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	//思路
	/*
	* VkAttachmentDescription:描述一个颜色或者深度模板附着的结构，他并不是一个真正的附着，而是一种单纯的描述
	* VkAttachmentDescription1,VkAttachmentDescription2,VkAttachmentDescription3,VkAttachmentDescription4(深度模板)，告诉外界我要什么
	* VkAttachmentReference:说明本个SubPass需要的attachment中的一个，这一个数组的索引id是多少，这一个附着的图片期望格式是什么
	* VkSubpassDescription:用来填写一个子Pass的描述结构
	* VkSubpassDependency:用来描述不同子流程之间的依赖关系
	*/

	class SubPass {
	public:
		SubPass();
		~SubPass();

		void addColorAttachmentReference(const VkAttachmentReference& reference);

		void addInputAttachmentReference(const VkAttachmentReference& reference);

		void setResolvedAttachmentReference(const VkAttachmentReference& reference);

		void setDepthStencilAttachmentReference(const VkAttachmentReference& reference);

		void buildSubPassDescription();

		[[nodiscard]] VkSubpassDescription getSubPassDescription() const { return _subPassDescription; }

	private:
		VkSubpassDescription _subPassDescription{};
		std::vector<VkAttachmentReference> _colorAttachmentReference{};
		std::vector<VkAttachmentReference> _InputAttachmentReference{};
		VkAttachmentReference _depthStencilReference{};
		VkAttachmentReference _resolveReference{};
	};


	class RenderPass {
	public:
		using Ptr = std::shared_ptr<RenderPass>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<RenderPass>(device);
		}

		RenderPass(const Device::Ptr& device);

		~RenderPass();

		void addSubPass(const SubPass& subpass);

		void addDependency(const VkSubpassDependency& dependency);

		void addAttachment(const VkAttachmentDescription& description);
		
		void buildPrenderPass();

		[[nodiscard]] VkRenderPass getRenderPass() const { return _renderPass; }
	private:

		VkRenderPass _renderPass{ VK_NULL_HANDLE };
		std::vector<SubPass> _subPasses{};
		std::vector<VkSubpassDependency> _dependencies;
		std::vector<VkAttachmentDescription> _attachmentDescriptions;

		Device::Ptr _device;
	};
}