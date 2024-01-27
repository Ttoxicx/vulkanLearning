#include "window.h"
#include "../application.h"
#include "../camera.h"

namespace FF::Wrapper {

	static void windowResized(GLFWwindow* window, int width, int height) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->mWindowResized = true;

	}

	static void cursorPosCallBack(GLFWwindow* window, double xPos, double yPos) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		auto app = pUserData->mApp;
		if (!app.expired()) {
			auto appReal = app.lock();
			appReal->onMouseMove(xPos, yPos);
		}
	}

	Window::Window(const int& width, const int& height) {
		_width = width;
		_height = height;
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		_window = glfwCreateWindow(_width, _height, "VulkanWindow", NULL, NULL);
		if (!_window)
		{
			std::cerr << "ERROR: Field to create window " << std::endl;
		}

		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, windowResized);
		glfwSetCursorPosCallback(_window, cursorPosCallBack);
	}

	Window::~Window() {
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(_window);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}

	void Window::proccessEvent() {
		if (mApp.expired()) {
			return;
		}

		auto app = mApp.lock();

		if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS) {
			app->onKeyDown(Camera::CAMERA_MOVE::MOVE_FORWARD);
		}

		if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS) {
			app->onKeyDown(Camera::CAMERA_MOVE::MOVE_LEFT);
		}

		if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS) {
			app->onKeyDown(Camera::CAMERA_MOVE::MOVE_BACKWARD);
		}

		if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS) {
			app->onKeyDown(Camera::CAMERA_MOVE::MOVE_RIGHT);
		}

		if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, true);
			return;
		}
		return;
	}
}