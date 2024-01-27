#include "command_pool.h"

namespace FF::Wrapper {
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		_device = device;
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();

		//ָ���޸ĵ����ԣ�ָ��ص��ڴ�����
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:���������CommandBuffer���Ե������£���������
		//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:ÿһ��CommandBuffer���ܵ���reset�����뼯�����ã�vkResetCommandPool
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