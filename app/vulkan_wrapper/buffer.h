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
		* ����Buffer�ķ���
		* 1 ͨ���ڴ�Mapping����ʽ��ֱ�Ӷ��ڴ���и��ģ�������HostVisible(CPU�ɼ�)���͵��ڴ�
		* 2 ����ڴ���LocalOptimal(��GPU�ɼ�)����ô���봴���м��StageBuffer���ȸ��Ƶ�StageBuffer���ٿ��뵽Ŀ��Buffer
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