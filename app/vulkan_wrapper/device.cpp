#include "device.h"

namespace FF::Wrapper {
	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface) {
		_instance = instance;
		_surface = surface;
		pickPhysicalDevice();
		initQueueFamilies(_physicalDevice);
		createLogicalDevice();
	}

	Device::~Device() {
		vkDestroyDevice(_device, nullptr);
	}

	void Device::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("Error: failed to enumerate physical device");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : devices) {
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}
		while (!candidates.empty() && _physicalDevice == VK_NULL_HANDLE) {
			if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {
				_physicalDevice = candidates.rbegin()->second;
				candidates.erase(candidates.rbegin()->first);
			}
		}

		if (_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Error: failed to get physical device");
		}
	}

	int Device::rateDevice(VkPhysicalDevice device) {
		int score = 0;

		//设备名称 类型 支持vulkan的版本
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		//纹理压缩 浮点数运算特性 多视口渲染
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		return score;
	}

	bool Device::isDeviceSuitable(VkPhysicalDevice device) {
		//设备名称 类型 支持vulkan的版本
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		//纹理压缩 浮点数运算特性 多视口渲染
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && deviceFeatures.samplerAnisotropy;
	}

	void Device::initQueueFamilies(VkPhysicalDevice device) {
		uint32_t qFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(device, &qFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> qFamilies(qFamilyCount);

		vkGetPhysicalDeviceQueueFamilyProperties(device, &qFamilyCount, qFamilies.data());

		int i = 0;
		for (const auto& queueFamily : qFamilies) {

			//寻找支持渲染的队列族
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				_graphicQueueFamily = i;
			}

			//寻找支持显示的队列族
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface->getSurface(), &presentSupport);
			if (presentSupport) {
				_presentQueueFamily = i;
			}

			if (isQueueFamilyComplete()) {
				break;
			}

			++i;
		}
	}

	void Device::createLogicalDevice() {

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> queueFamilies = { _graphicQueueFamily.value(),_presentQueueFamily.value() };

		float queuePriority = 1.0;

		for (uint32_t queueFamily : queueFamilies) {
			//填写创建信息
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//填写逻辑设备创建信息
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;//打开各项异性

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequredExtensions.data();

		//layer层
		if (_instance->getEnableValidationLayer()) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}
		if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create logical device");
		}

		vkGetDeviceQueue(_device, _graphicQueueFamily.value(), 0, &_graphicQueue);
		vkGetDeviceQueue(_device, _presentQueueFamily.value(), 0, &_presentQueue);
	}

	bool Device::isQueueFamilyComplete() {
		return _graphicQueueFamily.has_value() && _presentQueueFamily.has_value();
	}

	VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(_physicalDevice, &props);

		VkSampleCountFlags counts = std::min(
			props.limits.framebufferColorSampleCounts,
			props.limits.framebufferDepthSampleCounts
		);

		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
		return VK_SAMPLE_COUNT_1_BIT;
	}
}