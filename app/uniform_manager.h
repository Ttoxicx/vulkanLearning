#pragma once

#include "vulkan_wrapper/device.h"
#include "vulkan_wrapper/swap_chain.h"
#include "vulkan_wrapper/buffer.h"
#include "vulkan_wrapper/descriptor_set_layout.h"
#include "vulkan_wrapper/descriptor_pool.h"
#include "vulkan_wrapper/descriptor_set.h"
#include "vulkan_wrapper/descriptor.h"
#include "vulkan_wrapper/device.h"
#include "vulkan_wrapper/command_pool.h"
#include "base.h"

class UniformManager {
public:
	using Ptr = std::shared_ptr<UniformManager>;
	static Ptr create() { return std::make_shared<UniformManager>(); }

	UniformManager();

	~UniformManager();

	void init(
		const FF::Wrapper::Device::Ptr& device, 
		const FF::Wrapper::CommandPool::Ptr& commandPool, 
		int frameCount
	);

	void update(const VPMatrices& vpMatrices, const ObjectUniform& objUnifom, int frameCount);

	[[nodiscard]] FF::Wrapper::DescriptorSetLayout::Ptr getDescriptorSetLayout() const { return _descriptorSetLayout; }
	
	[[nodiscard]] FF::Wrapper::DescriptorPool::Ptr getDescriptorPool() const { return _descriptorPool; }

	[[nodiscard]] const VkDescriptorSet& getDescriptorSet(int frameCount) const { return _descrptorSet->getDescriptorSet(frameCount); }

private:

	std::vector<FF::Wrapper::UniformParameter::Ptr> _uniformParams;

	FF::Wrapper::DescriptorSetLayout::Ptr _descriptorSetLayout{ nullptr };
	FF::Wrapper::DescriptorPool::Ptr _descriptorPool{ nullptr };
	FF::Wrapper::DescriptorSet::Ptr _descrptorSet{ nullptr };

	FF::Wrapper::Device::Ptr _device{ nullptr };
};