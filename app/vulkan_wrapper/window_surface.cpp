#include "window_surface.h"

namespace FF::Wrapper {

	WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window) {
		_instance = instance;
		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &_surface) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create surface");
		}
	}

	WindowSurface::~WindowSurface() {
		vkDestroySurfaceKHR(_instance->getInstance(), _surface, nullptr);
	}
}