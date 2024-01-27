#pragma once

#include "../base.h"
#include "device.h"
#include "command_pool.h"

namespace FF::Wrapper {
	class CommandBuffer {
	public:
		using Ptr = std::shared_ptr<CommandBuffer>;
		static Ptr create(
			const Device::Ptr& device, 
			const CommandPool::Ptr& commandPool, 
			bool asSecondary = false
		) {
			return std::make_shared<CommandBuffer>(device, commandPool, asSecondary);
		}

		CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false);
		
		~CommandBuffer();

		//begin
		//xxx
		//beginRenderPass
		//xxxx �󶨸���ʵ������
		/*
		* while(n<objectNumber){
		*	object[n]->recordCommand()
		*	++n;
		* }
		*/
		//endRenderPass
		//end
		


		//VkCommandBufferUsageFlags:
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:�������ֻ�ᱻ�ύʹ��һ��
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:�������壬��һ���������壬λ��һ��renderPass����
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:�����Ѿ����ύ�ˣ�ִ���ڼ�����ٴα��ύ

		//VkCommandBufferInheritanceInfo:�����������Ƕ������壬��ô����ṹ�壬��¼������������������Ϣ/�̳���Ϣ
		void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});

		//VkSubpassContents:
		//VK_SUBPASS_CONTENTS_INLINE:��Ⱦָ��ᱻ��¼���������壬�������ɶ������������
		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:��Ⱦָ������˶���ָ����У�����������������beginRenderPass��ʹ���˶��������������ʹ��
		void beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents = VK_SUBPASS_CONTENTS_INLINE);

		void bindGraphicPipeline(const VkPipeline& pipeline);

		void bindVertexBuffer(const std::vector<VkBuffer>& buffers);

		void bindIndexBuffer(const VkBuffer& buffer);

		void bindDescriptorSet(const VkPipelineLayout& layout, const VkDescriptorSet& descriptorSet);

		void draw(size_t vertexCount);

		void drawIndex(size_t indexCount);

		void endRenderPass();

		void end();

		void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos);

		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height);

		void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, const VkPipelineStageFlags& srcStageMask, const VkPipelineStageFlags& dstStageMask);

	public:

		[[nodiscard]] VkCommandBuffer getCommandBuffer() const { return _commandBuffer; }

	private:
		VkCommandBuffer _commandBuffer{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
		CommandPool::Ptr _commandPool{ nullptr };
	};
}