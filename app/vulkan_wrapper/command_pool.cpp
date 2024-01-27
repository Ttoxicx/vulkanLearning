#include "command_pool.h"

namespace FF::Wrapper {
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		_device = device;
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();

		//指令修改的属性，指令池的内存属性
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:分配出来的CommandBuffer可以单独更新，单独重置
		//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:每一个CommandBuffer不能单独reset，必须集体重置，vkResetCommandPool
		createInfo.flags = flag;

		if (vkCreateCommandPool(_device->getDevice(), &createInfo, nullptr, &_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create command pool");
		}
	}
	CommandPool::~CommandPool() {
		if (_commandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(_device->getDevice(), _commandPool, nullptr);
		}
	}
}