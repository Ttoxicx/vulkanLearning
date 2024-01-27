#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	class Buffer {
	public:
		using Ptr = std::shared_ptr<Buffer>;
		static Ptr create(
			const Device::Ptr& device, 
			VkDeviceSize size, 
			VkBufferUsageFlags usage, 
			VkMemoryPropertyFlags properties
		) {
			return std::make_shared<Buffer>(device, size, usage, properties);
		}

		Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~Buffer();

		/*
		* 更新Buffer的方法
		* 1 通过内存Mapping的形式，直接对内存进行更改，适用于HostVisible(CPU可见)类型的内存
		* 2 如果内存是LocalOptimal(仅GPU可见)，那么必须创建中间的StageBuffer，先复制到StageBuffer，再考入到目标Buffer
		*/

		void updateBufferByMap(void* data, size_t size);

		void updateBufferByStage(void* data, size_t size);

		void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

	public:

		[[nodiscard]] VkBuffer getBuffer() const { return _buffer; }

		[[nodiscard]] const VkDescriptorBufferInfo& getDescriptorBufferInfo() const { return _bufferInfo; }
	public:

		static Ptr createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* data);

		static Ptr createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* data);

		static Ptr createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* data = nullptr);

		static Ptr createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* data = nullptr);

	private:
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);
	private:
		VkBuffer _buffer{ VK_NULL_HANDLE };
		VkDeviceMemory _bufferMemory{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
		VkDescriptorBufferInfo _bufferInfo{};
	};
}