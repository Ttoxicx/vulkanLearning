#pragma once

#include "../base.h"
#include "device.h"
#include "command_pool.h"

namespace FF::Wrapper {

	class Image {
	public:
		using Ptr = std::shared_ptr<Image>;

		static Ptr createDepthImage(
			const Device::Ptr& device,
			const uint32_t& width,
			const uint32_t& height,
			VkSampleCountFlagBits samples
		);

		static Ptr createRenderTargetImage(
			const Device::Ptr& device,
			const uint32_t& width,
			const uint32_t& height,
			VkFormat format,
			VkSampleCountFlagBits samples
		);

		static VkFormat findDepthFormat(const Device::Ptr& device);

		static VkFormat findSupportedFormat(
			const Device::Ptr& device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);

	public:

		static Ptr create(
			const Device::Ptr& device,
			const uint32_t& width,
			const uint32_t& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& imageTiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& samples,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags
		) {
			return std::make_shared<Image>(device, width, height, format, imageType, imageTiling, usage, samples, properties, aspectFlags);
		}

		Image(
			const Device::Ptr& device,
			const uint32_t& width,
			const uint32_t& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& imageTiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& samples,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags
		);

		
		~Image();

		//建议将该方法写入到工具类中
		void setImageLayout(
			const VkImageLayout& layout, 
			const VkPipelineStageFlags& srcStageMask, 
			const VkPipelineStageFlags& dstStageMask,
			const VkImageSubresourceRange& subresourceRange,
			const CommandPool::Ptr& commandPool
			);

		void fillImageData(const CommandPool::Ptr& commadPool, size_t size, void* data);

		bool hasStencilComponent();

		[[nodiscard]] VkImage getImage() const { return _image; }

		[[nodiscard]] VkImageView getImageView() const { return _imageView; }

		[[nodiscard]] VkImageLayout getLayout() const { return _layout; }

		[[nodiscard]] size_t getWidth() const { return _width; }

		[[nodiscard]] size_t getHeight() const { return _height; }

	private:

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		Device::Ptr _device{ nullptr };

		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
		VkImage _image{ VK_NULL_HANDLE };
		VkDeviceMemory _imageMemory{ VK_NULL_HANDLE };
		VkImageView _imageView{ VK_NULL_HANDLE };
		VkImageLayout _layout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkFormat _format;
	};

}