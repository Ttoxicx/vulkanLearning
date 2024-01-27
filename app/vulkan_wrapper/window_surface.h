#pragma once

#include "../base.h"
#include "instance.h"
#include "window.h"

namespace FF::Wrapper {
	class WindowSurface {
	public:
		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, Window::Ptr window) { 
			return std::make_shared<WindowSurface>(instance, window);
		}

		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		[[nodiscard]] VkSurfaceKHR getSurface() const { return _surface; }

	private:
		VkSurfaceKHR _surface{ VK_NULL_HANDLE };
		Instance::Ptr _instance{ nullptr };
	};
}