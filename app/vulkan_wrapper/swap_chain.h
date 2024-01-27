#pragma once

#include "../base.h"
#include "device.h"
#include "window.h"
#include "window_surface.h"
#include "render_pass.h"
#include "image.h"
#include "command_pool.h"

namespace FF::Wrapper {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SwapChain {
	public:

		using Ptr = std::shared_ptr<SwapChain>;

		static Ptr create(
			const Device::Ptr& device,
			const CommandPool::Ptr& commandPool,
			const Window::Ptr& window,
			const WindowSurface::Ptr& surface
		) {
			return std::make_shared<SwapChain>(device, commandPool, window, surface);
		}

		SwapChain(
			const Device::Ptr& device,
			const CommandPool::Ptr& commandPool,
			const Window::Ptr& window,
			const WindowSurface::Ptr& surface
		);
		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void createFrameBuffers(const RenderPass::Ptr& renderPass);

	public:

		[[nodiscard]] VkFormat getFormat() const { return _swapChainFormat; }

		[[nodiscard]] uint32_t getImageCount() const { return _imageCount; }

		[[nodiscard]] VkSwapchainKHR getSwapChain() const { return _swapChain; }

		[[nodiscard]] VkFramebuffer getFrameBuffer(const int index) { return _swapChainFrameBuffers[index]; }

		[[nodiscard]] VkExtent2D getExtent() const { return _swapChainExtent; }

	private:

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

	private:
		VkSwapchainKHR _swapChain{ VK_NULL_HANDLE };

		VkFormat _swapChainFormat;
		VkExtent2D _swapChainExtent;

		uint32_t _imageCount{ 0 };

		//VkImage 由SwapChain创建，销毁也要交给SwapChain
		std::vector<VkImage> _swapChainImages{};
		
		//对图像的管理器，管理框架
		std::vector<VkImageView> _swapChainImageViews{};

		std::vector<VkFramebuffer> _swapChainFrameBuffers{};

		//深度图片
		std::vector<Image::Ptr> _depthImages{};

		//多重采样，中间图片
		std::vector<Image::Ptr> _multiSampleImages{};

		Device::Ptr _device{ nullptr };
		Window::Ptr _window{ nullptr };
		WindowSurface::Ptr _surface{ nullptr };
	};
}