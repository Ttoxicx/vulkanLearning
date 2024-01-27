#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	class Semaphore {
	public:

		using Ptr = std::shared_ptr<Semaphore>;

		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<Semaphore>(device);
		}

		Semaphore(const Device::Ptr& device);

		~Semaphore();

		[[nodiscard]] VkSemaphore getSemaphore() const { return _semaphore; }

	private:
		VkSemaphore _semaphore{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
	};
}