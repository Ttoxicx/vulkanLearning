#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	//˼·
	/*
	* VkAttachmentDescription:����һ����ɫ�������ģ�帽�ŵĽṹ����������һ�������ĸ��ţ�����һ�ֵ���������
	* VkAttachmentDescription1,VkAttachmentDescription2,VkAttachmentDescription3,VkAttachmentDescription4(���ģ��)�����������Ҫʲô
	* VkAttachmentReference:˵������SubPass��Ҫ��attachment�е�һ������һ�����������id�Ƕ��٣���һ�����ŵ�ͼƬ������ʽ��ʲô
	* VkSubpassDescription:������дһ����Pass�������ṹ
	* VkSubpassDependency:����������ͬ������֮���������ϵ
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