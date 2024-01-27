#pragma once

#include "../base.h"
#include "instance.h"
#include "window_surface.h"

namespace FF::Wrapper {
	const std::vector<const char*> deviceRequredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) { 
			return std::make_shared<Device>(instance, surface);
		}

		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		void initQueueFamilies(VkPhysicalDevice device);

		void createLogicalDevice();

		bool isQueueFamilyComplete();

		VkSampleCountFlagBits getMaxUsableSampleCount();

		[[nodiscard]] VkDevice getDevice() const { return _device; }
		[[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return _physicalDevice; }
		[[nodiscard]] std::optional<uint32_t> getGraphicQueueFamily() const { return _graphicQueueFamily; }
		[[nodiscard]] std::optional<uint32_t> getPresentQueueFamily() const { return _presentQueueFamily; }
		[[nodiscard]] VkQueue getGraphicQueue() const { return _graphicQueue; }
		[[nodiscard]] VkQueue getPresentQueue() const { return _presentQueue; }
	private:
		VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
		Instance::Ptr _instance{ nullptr };
		WindowSurface::Ptr _surface{ nullptr };

		//存储当前渲染任务队列族的id
		std::optional<uint32_t> _graphicQueueFamily;
		VkQueue _graphicQueue{ VK_NULL_HANDLE };

		std::optional<uint32_t> _presentQueueFamily;
		VkQueue _presentQueue{ VK_NULL_HANDLE };

		//逻辑设备
		VkDevice _device{ VK_NULL_HANDLE };
	};
}