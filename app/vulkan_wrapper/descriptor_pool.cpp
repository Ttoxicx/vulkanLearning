#include "descriptor_pool.h"

namespace FF::Wrapper {

	DescriptorPool::DescriptorPool(const Device::Ptr& device) {
		_device = device;
	}

	DescriptorPool::~DescriptorPool() {
		if (_pool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(_device->getDevice(), _pool, nullptr);
		}
	}

	void DescriptorPool::build(std::vector<UniformParameter::Ptr>& params, const int frameCount) {
		
		
		int uniformBufferCount = 0;
		int textureCount = 0;
		
		for (const auto& param : params) {
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				++uniformBufferCount;
			}

			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				++textureCount;
			}
		}

		//描述每一种uniform都有多少
		std::vector<VkDescriptorPoolSize> poolSizes{};

		VkDescriptorPoolSize uniformBufferSize{};
		uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
		poolSizes.push_back(uniformBufferSize);

		VkDescriptorPoolSize textureSize{};
		textureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureSize.descriptorCount = textureCount * frameCount;//这边的size是指有多少个descriptor
		poolSizes.push_back(textureSize);

		//创建pool
		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.maxSets = static_cast<uint32_t>(frameCount);

		if (vkCreateDescriptorPool(_device->getDevice(), &createInfo, nullptr, &_pool) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create descriptor pool");
		}
	}

}