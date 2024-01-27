#include "image.h"
#include "command_buffer.h"
#include "buffer.h"

namespace FF::Wrapper {

	Image::Ptr Image::createDepthImage(
		const Device::Ptr& device,
		const uint32_t& width,
		const uint32_t& height,
		VkSampleCountFlagBits samples
	) {
		std::vector<VkFormat> formats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		VkFormat resultFormat = findSupportedFormat(
			device, formats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		return Image::create(
			device,
			width, height,
			resultFormat,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			samples,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);
	}

	Image::Ptr Image::createRenderTargetImage(
		const Device::Ptr& device,
		const uint32_t& width,
		const uint32_t& height,
		VkFormat format,
		VkSampleCountFlagBits samples
	){
		return Image::create(
			device,
			width, height,
			format,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,

			//��ΪVK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			//ֻ������ʹ�ñ�ͼƬ��ʱ�򣬲Ż�Ϊ�䴴���ڴ棬����
			//һ��������transient,�ͱ�����DeviceMemory���ɵ�ʱ�򣬼�һ����������lazy
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			samples,

			//���Ϸ�ʹ����VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			//���������Ҫ����VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	Image::Image(
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
		_device = device;
		_width = width;
		_height = height;
		_format = format;
		_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.format = format; //rgb rgba ...
		imageCreateInfo.imageType = imageType;
		imageCreateInfo.tiling = imageTiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.samples = samples;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(_device->getDevice(), &imageCreateInfo, nullptr, &_image) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image");
		}

		//�����ڴ�ռ�
		VkMemoryRequirements memReq{};
		vkGetImageMemoryRequirements(_device->getDevice(), _image, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;

		//��������buffer������ڴ����͵�ID��
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(_device->getDevice(), &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate memory");
		}

		vkBindImageMemory(_device->getDevice(), _image, _imageMemory, 0);

		//����imageView
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = imageType == VK_IMAGE_TYPE_2D ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.image = _image;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(_device->getDevice(), &imageViewCreateInfo, nullptr, &_imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view");
		}

	}

	Image::~Image() {
		if (_imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(_device->getDevice(), _imageView, nullptr);
		}
		if (_imageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(_device->getDevice(), _imageMemory, nullptr);
		}
		if (_image != VK_NULL_HANDLE) {
			vkDestroyImage(_device->getDevice(), _image, nullptr);
		}
	}

	void Image::setImageLayout(
		const VkImageLayout& layout,
		const VkPipelineStageFlags& srcStageMask,
		const VkPipelineStageFlags& dstStageMask,
		const VkImageSubresourceRange& subresourceRange,
		const CommandPool::Ptr& commandPool
	) {
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = _layout;
		imageMemoryBarrier.newLayout = layout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = _image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		//���ȴ��Ĳ���
		switch (_layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageMemoryBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		default:
			break;
		}

		//�������Ĳ���
		switch (layout) 
		{
			//���Ŀ���ǽ�ͼƬת��Ϊһ�����Ʋ�����Ŀ��ͼƬ����ô�������Ĳ���һ����д�����
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
			//���Ŀ���ǽ�ͼƬת��Ϊһ���ʺϱ���Ϊ����ĸ�ʽ����ô�������Ĳ���һ���Ƕ�ȡ
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (imageMemoryBarrier.srcAccessMask == 0) {
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		default:
			break;
		}

		_layout = layout;

		auto commandBuffer = CommandBuffer::create(_device, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
		commandBuffer->end();
		commandBuffer->submitSync(_device->getGraphicQueue());

	}

	void Image::fillImageData(const CommandPool::Ptr& commadPool, size_t size, void* data) {
		assert(data);
		assert(size);
		auto stageBuffer = Buffer::createStageBuffer(_device, size, data);
		auto commandBuffer = CommandBuffer::create(_device, commadPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), _image, _layout, _width, _height);
		commandBuffer->end();
		commandBuffer->submitSync(_device->getGraphicQueue());
	}

	uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(_device->getPhysicalDevice(), &memProps);

		//���� typeFilter=0x0001|0x0100 ��i==0ʱ (typeFilter & (1 << i) ���Ϊ�棬
		//typeFilter�����Ϊ�±�Ļ�ֻ����������±�Ϊ ��1<<i��

		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
			if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}
		throw std::runtime_error("Error: failed to find the property memory type");
	}

	VkFormat Image::findDepthFormat(const Device::Ptr& device) {
		std::vector<VkFormat> formats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		VkFormat resultFormat = findSupportedFormat(
			device, formats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		return resultFormat;
	}

	VkFormat Image::findSupportedFormat(
		const Device::Ptr& device,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	) {
		for (auto format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("Error: failed to find proper format");
	}

	bool Image::hasStencilComponent() {
		return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}