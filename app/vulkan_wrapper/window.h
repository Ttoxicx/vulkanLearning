#pragma once

#include "../base.h"

namespace FF {
	class Application;
}

namespace FF::Wrapper {

	class Window {
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) { 
			return std::make_shared<Window>(width, height);
		}

		Window(const int& width,const int& height);

		~Window();

		bool shouldClose();
		
		void pollEvents();

		void proccessEvent();

		void setApp(std::shared_ptr<Application> app) { mApp = app; }

		[[nodiscard]] GLFWwindow* getWindow() const { return _window; }

	public:
		bool mWindowResized{ false };
		std::weak_ptr<Application> mApp;
	private:
		int _width{ 0 };
		int _height{ 0 };
		GLFWwindow* _window{ NULL };
	};
}