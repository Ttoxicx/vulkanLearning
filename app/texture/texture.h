#pragma once

#include "../base.h"
#include "../vulkan_wrapper/image.h"
#include "../vulkan_wrapper/sampler.h"

namespace FF {

	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;

		static Ptr create(
			const Wrapper::Device::Ptr& device, 
			const Wrapper::CommandPool::Ptr& commandPool, 
			const std::string& imageFilePath
		) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(
			const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			const std::string& imageFilePath
		);

		~Texture();

		[[nodiscard]] Wrapper::Image::Ptr getImage() const { return _image; }

		[[nodiscard]] Wrapper::Sampler::Ptr getSampler() const { return _sampler; }

		[[nodiscard]] const VkDescriptorImageInfo& getImageInfo() { return _imageInfo; }

	private:
		Wrapper::Device::Ptr _device{ nullptr };
		Wrapper::Image::Ptr _image{ nullptr };
		Wrapper::Sampler::Ptr _sampler{ nullptr };
		VkDescriptorImageInfo _imageInfo{};
	};
}