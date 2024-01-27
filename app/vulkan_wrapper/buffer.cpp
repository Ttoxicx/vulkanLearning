#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"

namespace FF::Wrapper {

	Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
		_device = device;

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(_device->getDevice(), &createInfo, nullptr, &_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create buffer");
		}

		//创建显存空间
		VkMemoryRequirements memReq{};
		vkGetBufferMemoryRequirements(_device->getDevice(), _buffer, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;

		//符合上述buffer需求的内存类型的ID们
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(_device->getDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate memory");
		}

		vkBindBufferMemory(_device->getDevice(), _buffer, _bufferMemory, 0);

		_bufferInfo.buffer = _buffer;
		_bufferInfo.offset = 0;
		_bufferInfo.range = size;
	}

	Buffer::~Buffer() {
		if (_buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(_device->getDevice(), _buffer, nullptr);
		}
		if (_bufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(_device->getDevice(), _bufferMemory, nullptr);
		}

	}

	uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(_device->getPhysicalDevice(), &memProps);

		//例如 typeFilter=0x0001|0x0100 当i==0时 (typeFilter & (1 << i) 结果为真，
		//typeFilter可理解为下标的或，只不过这里的下标为 （1<<i）

		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
			if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}
		throw std::runtime_error("Error: failed to find the property memory type");
	}

	void Buffer::updateBufferByMap(void* data, size_t size) {
		void* memPtr = nullptr;
		vkMapMemory(_device->getDevice(), _bufferMemory, 0, size, 0, &memPtr);
		memcpy(memPtr, data, size);
		vkUnmapMemory(_device->getDevice(), _bufferMemory);
	}

	void Buffer::updateBufferByStage(void* data, size_t size) {
		auto stageBuffer = Buffer::create(
			_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		stageBuffer->updateBufferByMap(data, size);
		copyBuffer(stageBuffer->getBuffer(), _buffer, static_cast<VkDeviceSize>(size));
	}

	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size) {
		auto commandPool = CommandPool::create(_device);
		auto commandBuffer = CommandBuffer::create(_device, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy copyInfo{};
		copyInfo.size = size;
		commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, 1, { copyInfo });
		commandBuffer->end();
		commandBuffer->submitSync(_device->getGraphicQueue(), VK_NULL_HANDLE);
	}

	Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* data) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		buffer->updateBufferByStage(data, size);
		return buffer;
	}

	Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* data) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		buffer->updateBufferByStage(data, size);
		return buffer;
	}

	Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* data) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		if (data != nullptr) {
			//buffer->updateBufferByStage(data, size);
			buffer->updateBufferByMap(data, size);
		}
		return buffer;
	}

	Buffer::Ptr Buffer::createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* data) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		if (data != nullptr) {
			//buffer->updateBufferByStage(data, size);
			buffer->updateBufferByMap(data, size);
		}
		return buffer;
	}

}