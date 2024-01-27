#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

namespace FF {
	Texture::Texture(
		const Wrapper::Device::Ptr& device, 
		const Wrapper::CommandPool::Ptr& commandPool, 
		const std::string& imageFilePath
	) {
		_device = device;

		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load(imageFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			throw std::runtime_error("Error: failed to read image data");
		}

		_image = Wrapper::Image::create(
			_device, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,//只在GPU端被使用
			VK_IMAGE_ASPECT_COLOR_BIT
		);


		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.baseArrayLayer = 0;
		region.layerCount = 1;
		region.baseMipLevel = 0;
		region.levelCount = 1;

		_image->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			region, commandPool
		);
		size_t texSize = texWidth * texHeight * sizeof(int);
		_image->fillImageData(commandPool, texSize, (void*)pixels);

		_image->setImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			region, commandPool
		);

		stbi_image_free(pixels);

		_sampler = Wrapper::Sampler::create(_device);

		_imageInfo.imageLayout = _image->getLayout();
		_imageInfo.imageView = _image->getImageView();
		_imageInfo.sampler = _sampler->getSamper();
	}
	Texture::~Texture() {

	}
}