#include "descriptor_set.h"

namespace FF::Wrapper {
	DescriptorSet::DescriptorSet(
		const Device::Ptr& device,
		const std::vector<UniformParameter::Ptr> params,
		const DescriptorSetLayout::Ptr& layout,
		const DescriptorPool::Ptr& pool,
		int frameCount
	) {
		_device = device;
		std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool->getPool();
		allocInfo.descriptorSetCount = frameCount;
		allocInfo.pSetLayouts = layouts.data();
		
		_descriptorSets.resize(frameCount);
		if (vkAllocateDescriptorSets(_device->getDevice(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate descriptor sets");
		}

		for (int i = 0; i < frameCount; ++i) {
			//对每个DescriptorSet,我们需要把params里面的描述信息，写入其中
			std::vector<VkWriteDescriptorSet> descriptorSetWrites{};
			for (const auto& param : params) {
				VkWriteDescriptorSet descriptorSetWrite{};
				descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSetWrite.dstBinding = param->mBinding;
				descriptorSetWrite.dstSet = _descriptorSets[i];
				descriptorSetWrite.dstArrayElement = 0;
				descriptorSetWrite.descriptorType = param->mDescriptorType;
				descriptorSetWrite.descriptorCount = param->mCount;

				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					descriptorSetWrite.pBufferInfo = &(param->mBuffers[i]->getDescriptorBufferInfo());
				}

				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					descriptorSetWrite.pImageInfo = &(param->mTexture->getImageInfo());
				}

				descriptorSetWrites.push_back(descriptorSetWrite);
			}

			vkUpdateDescriptorSets(
				_device->getDevice(),
				static_cast<uint32_t>(descriptorSetWrites.size()),
				descriptorSetWrites.data(),
				0, nullptr
			);
		}
	}

	DescriptorSet::~DescriptorSet() {

	}
}