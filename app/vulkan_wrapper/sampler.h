#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	class Sampler {
	public:

		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<Sampler>(device); }

		Sampler(const Device::Ptr& device);

		~Sampler();

		[[nodiscard]] VkSampler getSamper() const { return _sampler; }

	private:

		Device::Ptr _device{ nullptr };

		VkSampler _sampler{ VK_NULL_HANDLE };

	};

}