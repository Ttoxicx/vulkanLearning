#include "command_buffer.h"

namespace FF::Wrapper {

	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary) {
		_device = device;
		_commandPool = commandPool;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = _commandPool->getCommandPool();
		allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (vkAllocateCommandBuffers(_device->getDevice(), &allocInfo, &_commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create command buffer");
		}
	}
	CommandBuffer::~CommandBuffer() {
		if (_commandBuffer != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(_device->getDevice(), _commandPool->getCommandPool(), 1, &_commandBuffer);
		}
	}

	void CommandBuffer::begin(
		VkCommandBufferUsageFlags flag, 
		const VkCommandBufferInheritanceInfo& inheritance
	) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flag;
		beginInfo.pInheritanceInfo = &inheritance;

		if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to begin command buffer");
		}
	}


	void CommandBuffer::beginRenderPass(
		const VkRenderPassBeginInfo& renderPassBeginInfo, 
		const VkSubpassContents& subPassContents
	) {
		vkCmdBeginRenderPass(_commandBuffer, &renderPassBeginInfo, subPassContents);
	}

	void CommandBuffer::bindGraphicPipeline(const VkPipeline& pipeline) {
		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers) {
		std::vector<VkDeviceSize> offsets(buffers.size(), 0);
		vkCmdBindVertexBuffers(
			_commandBuffer, 0,
			static_cast<uint32_t>(buffers.size()),
			buffers.data(), offsets.data()
		);
	}

	void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer) {
		vkCmdBindIndexBuffer(_commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout& layout, const VkDescriptorSet& descriptorSet) {
		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void CommandBuffer::draw(size_t vertexCount) {
		vkCmdDraw(_commandBuffer, vertexCount, 1, 0, 0);
	}

	void CommandBuffer::drawIndex(size_t indexCount) {
		vkCmdDrawIndexed(_commandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::endRenderPass() {
		vkCmdEndRenderPass(_commandBuffer);
	}

	void CommandBuffer::end() {
		if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to end command buffer");
		}
	}

	void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos) {
		vkCmdCopyBuffer(_commandBuffer, srcBuffer, destBuffer, copyInfoCount, copyInfos.data());
	}

	void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height) {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;

		//为0代表不需要padding
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0,0,0 };
		region.imageExtent = { width,height,1 };

		vkCmdCopyBufferToImage(_commandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &region);
	}

	void CommandBuffer::submitSync(VkQueue queue, VkFence fence) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkQueueWaitIdle(queue);
	}

	void CommandBuffer::transferImageLayout(
		const VkImageMemoryBarrier& imageMemoryBarrier,
		const VkPipelineStageFlags& srcStageMask,
		const VkPipelineStageFlags& dstStageMask) {

		vkCmdPipelineBarrier(
			_commandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,//memory barrier
			0, nullptr,//buffer memory barrier
			1, &imageMemoryBarrier//image memory barrier
		);
	}
}