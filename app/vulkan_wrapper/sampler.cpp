#include "sampler.h"

namespace FF::Wrapper {

	Sampler::Sampler(const Device::Ptr& device) {
		_device = device;

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.anisotropyEnable = VK_TRUE;
		createInfo.maxAnisotropy = 16;
		createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//我们是否使用非归一化的uvw坐标体系0-1
		createInfo.unnormalizedCoordinates = VK_FALSE;
		//采样得到的值是否需要和一个值进行比较，比较方式可以设置，通过了比较测试才采样，否则不采
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.mipLodBias = 0.0f;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 0.0f;
		if (vkCreateSampler(_device->getDevice(), &createInfo, nullptr, &_sampler) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create sampler");
		}
	}

	Sampler::~Sampler() {
		if (_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(_device->getDevice(), _sampler, nullptr);
		}
	}
}