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

		//选择vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.formats);

		//选择presentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.presentModes);

		//选择交换链范围
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.capabilities);

		//设置图像缓冲数量
		_imageCount = swapChainSupportInfo.capabilities.minImageCount + 1;

		//如果maxImageCount为0，说明只要内存不爆炸，我们就可以设定任意数量的images
		if (swapChainSupportInfo.capabilities.maxImageCount > 0 && _imageCount > swapChainSupportInfo.capabilities.maxImageCount) {
			_imageCount = swapChainSupportInfo.capabilities.maxImageCount;
		}

		//填写创建信息,此处初始或必须置为空，因为会有忘记设置的变量，值为随机
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _surface->getSurface();
		createInfo.minImageCount = _imageCount;//我现在设置的数量，适合当前情况，但可能会得到更多
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		
		//图像包含的层次，VR一般会有两个
		createInfo.imageArrayLayers = 1;

		//交换链生成的图像，到底用于何处
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		//因为交换链的图像，会被用来渲染或显示，而渲染跟显示的分别使用不同的队列，所以会出现两个队列使用同一个交换链的情况
		//那么我们就需要设置，让交换链的图像，被两个队列使用兼容
		
		std::vector<uint32_t> queueFamilies = { _device->getGraphicQueueFamily().value(),_device->getPresentQueueFamily().value() };
		if (_device->getGraphicQueueFamily().value() == _device->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//被某一个队列族独占，性能会更好
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//可以被共享的模式
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//交换链的图像初始变化，比如是否需要反转
		createInfo.preTransform = swapChainSupportInfo.capabilities.currentTransform;
		
		//不与原来窗体当中的内容混合
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//当前窗体被挡住的部分，不用绘制，但是会影响到回读！
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(_device->getDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create swap chain");
		}

		_swapChainFormat = surfaceFormat.format;
		_swapChainExtent = extent;

		//系统可能创建更多的image，当前的imageCount是最小数量
		vkGetSwapchainImagesKHR(_device->getDevice(), _swapChain, &_imageCount, nullptr);
		_swapChainImages.resize(_imageCount);

		vkGetSwapchainImagesKHR(_device->getDevice(), _swapChain, &_imageCount, _swapChainImages.data());

		//创建imageView
		_swapChainImageViews.resize(_imageCount);
		for (int i = 0; i < _imageCount; ++i) {
			_swapChainImageViews[i] = createImageView(_swapChainImages[i], _swapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		//创建depthImage
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

		//创建multiSampleImage
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
		//获取基础特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &info.capabilities);

		//获取表面支持格式
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device->getPhysicalDevice(), _surface->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			info.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(_device->getPhysicalDevice(), _surface->getSurface(), &formatCount, info.formats.data());
		}

		//获取呈现模式
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &presentModeCount, nullptr);
		
		if (presentModeCount != 0) {
			info.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(_device->getPhysicalDevice(), _surface->getSurface(), &presentModeCount, info.presentModes.data());
		}

		return info;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		//如果只返回了一个未定义的格式，那么就没有首选格式，我们自己做一个
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
		//在设备上，只有FIFO被绝对支持，如果在移动设备上，为了节省电源，优先选择FIFO
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
		//如果出现以下情况，说明系统不让我们自己设定extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		//由于高清屏幕情况下，比如苹果，窗体的坐标大小，并不等于像素的长宽
		int width = 0, height = 0;
		glfwGetFramebufferSize(_window->getWindow(), &width, &height);

		VkExtent2D actuallExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//规定在max与min之间

		actuallExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actuallExtent.width));
		actuallExtent.height = std::max(capabilities.maxImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actuallExtent.height));

		return actuallExtent;
	}

	void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass) {
		//创建FrameBuffer
		_swapChainFrameBuffers.resize(_imageCount);
		for (int i = 0; i < _imageCount; ++i) {
			//FrameBuffer为一帧的数据，比如n个ColorAttachment 1个DepthStencilAttachment,
			//这些东西的集合为一个FrameBuffer，那么这些东西送入管线就会形成一个GPU的集合，由上方的Attachment构成
			//注意数组中的顺序，必须与RenderPass匹配
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