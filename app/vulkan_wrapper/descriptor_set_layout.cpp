#include "descriptor_set_layout.h"

namespace FF::Wrapper {
	DescriptorSetLayout::DescriptorSetLayout(const Device::Ptr& device) {
		_device = device;
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		if (_layout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(_device->getDevice(), _layout, nullptr);
		}
	}

	void DescriptorSetLayout::build(const std::vector<UniformParameter::Ptr>& params) {
		if (_layout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(_device->getDevice(), _layout, nullptr);
		}
		_params = params;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

		for (const auto& param : _params) {
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.descriptorType = param->mDescriptorType;
			layoutBinding.binding = param->mBinding;
			layoutBinding.stageFlags = param->mStage;
			layoutBinding.descriptorCount = param->mCount;
			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		createInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(_device->getDevice(), &createInfo, nullptr, &_layout) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create descriptor set layout");
		}
	}
}