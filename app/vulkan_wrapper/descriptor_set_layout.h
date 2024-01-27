#pragma once

#include "../base.h"
#include "device.h"
#include "descriptor.h"

namespace FF::Wrapper {

	class DescriptorSetLayout {
	public:
		using Ptr = std::shared_ptr<DescriptorSetLayout>;

		static Ptr create(const Device::Ptr& device) { 
			return std::make_shared<DescriptorSetLayout>(device);
		}

		DescriptorSetLayout(const Device::Ptr& device);

		~DescriptorSetLayout();

		void build(const std::vector<UniformParameter::Ptr>& params);

		[[nodiscard]] auto getLayout() const { return _layout; }

	private:
		VkDescriptorSetLayout _layout{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
		std::vector<UniformParameter::Ptr> _params{};
	};
}