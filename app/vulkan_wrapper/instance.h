#pragma once

#include "../base.h"

namespace FF::Wrapper {
	class Instance {
	public:
		using Ptr = std::shared_ptr<Instance>;

		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		Instance(bool enableValidationLayer);

		~Instance();

		void printAvailableExtensions();

		std::vector<const char*> getRequiredExtensions(); 

		bool checkValidationLayerSupport();

		void setupDebugger();

		[[nodiscard]] VkInstance getInstance() const { return _instance; }

		[[nodiscard]] bool getEnableValidationLayer() const { return _enableValidationLayer; }
	private:
		VkInstance _instance{ VK_NULL_HANDLE };
		bool _enableValidationLayer{ false };
		VkDebugUtilsMessengerEXT _debugger{ VK_NULL_HANDLE };
	};
}