#include "semaphore.h"

namespace FF::Wrapper {

	Semaphore::Semaphore(const Device::Ptr & device) {
		_device = device;

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(_device->getDevice(), &createInfo, nullptr, &_semaphore) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create semaphore");
		}
	}

	Semaphore::~Semaphore() {
		if (_semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(_device->getDevice(), _semaphore, nullptr);
		}
	}

}