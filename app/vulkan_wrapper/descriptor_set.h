#pragma once

#include "../base.h"
#include "device.h"
#include "descriptor.h"
#include "descriptor_set_layout.h"
#include "descriptor_pool.h"

namespace FF::Wrapper {
	/*
	* 对于每一个模型的渲染，都需要绑定一个DescriptorSet, 绑定的位置就是在CommandBuffer
	* 一个DescriptorSet里面，都对应一个vp矩阵使用的buffer,一个model矩阵使用的buffer，等等，
	* 其中也包括bingSize等信息
	*由于交换链的存在，多帧可能并行渲染，所以我们需要为每一个交换链的图片，对应生成一个DescriptorSet
	*/

	class DescriptorSet {
	public:
		
		using Ptr = std::shared_ptr<DescriptorSet>;
		
		static Ptr create(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr> params,
			const DescriptorSetLayout::Ptr& layout,
			const DescriptorPool::Ptr& pool,
			int frameCount
		) {
			return std::make_shared<DescriptorSet>(device, params, layout, pool, frameCount);
		}

		DescriptorSet(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr> params,
			const DescriptorSetLayout::Ptr& layout,
			const DescriptorPool::Ptr& pool,
			int frameCount
		);

		~DescriptorSet();

		[[nodiscard]] const VkDescriptorSet& getDescriptorSet(int frameCount) const { return _descriptorSets.at(frameCount); }

	private:
		Device::Ptr _device{ nullptr };
		std::vector<VkDescriptorSet> _descriptorSets{};
	};

}