#include "fence.h"

namespace FF::Wrapper {
	Fence::Fence(const Device::Ptr& device, bool signaled) {
		_device = device;

		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		if (vkCreateFence(_device->getDevice(), &createInfo, nullptr, &_fence) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create fence");
		}
	}

	Fence::~Fence() {
		if (_fence != VK_NULL_HANDLE) {
			vkDestroyFence(_device->getDevice(), _fence, nullptr);
		}
	}

	void Fence::resetFence() {
		vkResetFences(_device->getDevice(), 1, &_fence);
	}

	void Fence::block(uint64_t timeout) {
		vkWaitForFences(_device->getDevice(), 1, &_fence, VK_TRUE, timeout);
	}
}