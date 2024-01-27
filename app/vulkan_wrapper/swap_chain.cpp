#include "swap_chain.h"

namespace FF::Wrapper {
	SwapChain::SwapChain(
		const Device::Ptr& device, 
		const CommandPool::Ptr& commandPool,
		const Window::Ptr& window, 
		const WindowSurface::Ptr& surface
	) {
		_device = device;
		_window = window;
		_surface = surface;

		auto swapChainSupportInfo = querySwapChainSupportInfo();

		//ѡ��vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.formats);

		//ѡ��presentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.presentModes);

		//ѡ�񽻻�����Χ
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.capabilities);

		//����ͼ�񻺳�����
		_imageCount = swapChainSupportInfo.capabilities.minImageCount + 1;

		//���maxImageCountΪ0��˵��ֻҪ�ڴ治��ը�����ǾͿ����趨����������images
		if (swapChainSupportInfo.capabilities.maxImageCount > 0 && _imageCount > swapChainSupportInfo.capabilities.maxImageCount) {
			_imageCount = swapChainSupportInfo.capabilities.maxImageCount;
		}

		//��д������Ϣ,�˴���ʼ�������Ϊ�գ���Ϊ�����������õı�����ֵΪ���
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _surface->getSurface();
		createInfo.minImageCount = _imageCount;//���������õ��������ʺϵ�ǰ����������ܻ�õ�����
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		
		//ͼ������Ĳ�Σ�VRһ���������
		createInfo.imageArrayLayers = 1;

		//���������ɵ�ͼ�񣬵������ںδ�
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		//��Ϊ��������ͼ�񣬻ᱻ������Ⱦ����ʾ������Ⱦ����ʾ�ķֱ�ʹ�ò�ͬ�Ķ��У����Ի������������ʹ��ͬһ�������������
		//��ô���Ǿ���Ҫ���ã��ý�������ͼ�񣬱���������ʹ�ü���
		
		std::vector<uint32_t> queueFamilies = { _device->getGraphicQueueFamily().value(),_device->getPresentQueueFamily().value() };
		if (_device->getGraphicQueueFamily().value() == _device->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//��ĳһ���������ռ�����ܻ����
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//���Ա������ģʽ
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//��������ͼ���ʼ�仯�������Ƿ���Ҫ��ת
		createInfo.preTransform = swapChainSupportInfo.capabilities.currentTransform;
		
		//����ԭ�����嵱�е����ݻ��
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//��ǰ���屻��ס�Ĳ��֣����û��ƣ����ǻ�Ӱ�쵽�ض���
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(_device->getDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create swap chain");
		}

		_swapChainFormat = surfaceFormat.format;
		_swapChainExtent = extent;

		//ϵͳ���ܴ��������image����ǰ��imageCount����С����
		vkGetSwapchainImagesKHR(_device->getDevice(), _swapChain, &_imageCount, nullptr);
		_swapChainImages.resize(_imageCount);

		vkGetSwapchainImagesKHR(_device->getDevice(), _swapChain, &_imageCount, _swapChainImages.data());

		//����imageView
		_swapChainImageViews.resize(_imageCount);
		for (int i = 0; i < _imageCount; ++i) {
			_swapChainImageViews[i] = createImageView(_swapChainImages[i], _swapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		//����depthImage
		_depthImages.resize(_imageCount);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		region.baseMipLevel = 0;
		region.levelCount = 1;
		region.baseArrayLayer = 0;
		region.layerCount = 1;

		for (int i = 0; i < _imageCount; ++i) {
			_depthImages[i] = Image::createDepthImage(
				_device, 
				_swapChainExtent.width, 
				_swapChainExtent.height,
				_device->getMaxUsableSampleCount()
			);
			_depthImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				region, commandPool
			);
		}

		//����multiSampleImage
		VkImageSubresourceRange mulsampleRegion{};
		mulsampleRegion.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mulsampleRegion.baseMipLevel = 0;
		mulsampleRegion.levelCount = 1;
		mulsampleRegion.baseArrayLayer = 0;
		mulsampleRegion.layerCount = 1;
		_multiSampleImages.resize(_imageCount);
		for (int i = 0; i < _imageCount; ++i) {
			_multiSampleImages[i] = Image::createRenderTargetImage(
				_device, _swapChainExtent.width,
				_swapChainExtent.height, _swapChainFormat,
				_device->getMaxUsableSampleCount()
			);
			_multiSampleImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				mulsampleRegion, commandPool
			);
		}
	}
	SwapChain::~SwapChain() {
		for (auto& imageView : _swapChainImageViews) {
			vkDestroyImageView(_device->getDevice(), imageView, nullptr);
		}

		for (auto& frameBuffer : _swapChainFrameBuffers) {
			vkDestroyFramebuffer(_device->getDevice(), frameBuffer, nullptr);
		}

		if (_swapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(_device->getDevice(), _swapChain, nullptr);
		}
	}

	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo info;
		//��ȡ��������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &info.capabilities);

		//��ȡ����֧�ָ�ʽ
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device->getPhysicalDevice(), _surface->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			info.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(_device->getPhysicalDevice(), _surface->getSurface(), &formatCount, info.formats.data());
		}

		//��ȡ����ģʽ
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &presentModeCount, nullptr);
		
		if (presentModeCount != 0) {
			info.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &presentModeCount, info.presentModes.data());
		}

		return info;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		//���ֻ������һ��δ����ĸ�ʽ����ô��û����ѡ��ʽ�������Լ���һ��
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_SRGB,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		//���豸�ϣ�ֻ��FIFO������֧�֣�������ƶ��豸�ϣ�Ϊ�˽�ʡ��Դ������ѡ��FIFO
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			/*else if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {

			}*/
		}

		return bestMode;
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		//����������������˵��ϵͳ���������Լ��趨extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		//���ڸ�����Ļ����£�����ƻ��������������С�������������صĳ���
		int width = 0, height = 0;
		glfwGetFramebufferSize(_window->getWindow(), &width, &height);

		VkExtent2D actuallExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//�涨��max��min֮��

		actuallExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actuallExtent.width));
		actuallExtent.height = std::max(capabilities.maxImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actuallExtent.height));

		return actuallExtent;
	}

	void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass) {
		//����FrameBuffer
		_swapChainFrameBuffers.resize(_imageCount);
		for (int i = 0; i < _imageCount; ++i) {
			//FrameBufferΪһ֡�����ݣ�����n��ColorAttachment 1��DepthStencilAttachment,
			//��Щ�����ļ���Ϊһ��FrameBuffer����ô��Щ����������߾ͻ��γ�һ��GPU�ļ��ϣ����Ϸ���Attachment����
			//ע�������е�˳�򣬱�����RenderPassƥ��
			std::array<VkImageView, 3> attachments = {
				_swapChainImageViews[i],
				_multiSampleImages[i]->getImageView(),
				_depthImages[i]->getImageView()
			};
			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = _swapChainExtent.width;
			frameBufferCreateInfo.height = _swapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(_device->getDevice(), &frameBufferCreateInfo, nullptr, &_swapChainFrameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error: failed to create frame buffer");
			}
		}
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(_device->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view in swap chain");
		}
		return imageView;
	}
}