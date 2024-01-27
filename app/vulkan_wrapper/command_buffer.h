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
		//xxxx 绑定各类实际数据
		/*
		* while(n<objectNumber){
		*	object[n]->recordCommand()
		*	++n;
		* }
		*/
		//endRenderPass
		//end
		


		//VkCommandBufferUsageFlags:
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:这个命令只会被提交使用一次
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:这个命令缓冲，是一个二级缓冲，位于一个renderPass当中
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:命令已经被提交了，执行期间可以再次被提交

		//VkCommandBufferInheritanceInfo:如果本命令缓冲是二级缓冲，那么这个结构体，记录了他所属的主命令信息/继承信息
		void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});

		//VkSubpassContents:
		//VK_SUBPASS_CONTENTS_INLINE:渲染指令会被记录在这个命令缓冲，本命令缓冲可定就是主命令缓冲
		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:渲染指令放在了二级指令缓冲中，试用于主命令缓冲调用beginRenderPass且使用了二级命令缓冲的情况下使用
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