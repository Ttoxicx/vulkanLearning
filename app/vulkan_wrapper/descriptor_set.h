#pragma once

#include "../base.h"
#include "device.h"
#include "descriptor.h"
#include "descriptor_set_layout.h"
#include "descriptor_pool.h"

namespace FF::Wrapper {
	/*
	* ����ÿһ��ģ�͵���Ⱦ������Ҫ��һ��DescriptorSet, �󶨵�λ�þ�����CommandBuffer
	* һ��DescriptorSet���棬����Ӧһ��vp����ʹ�õ�buffer,һ��model����ʹ�õ�buffer���ȵȣ�
	* ����Ҳ����bingSize����Ϣ
	*���ڽ������Ĵ��ڣ���֡���ܲ�����Ⱦ������������ҪΪÿһ����������ͼƬ����Ӧ����һ��DescriptorSet
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