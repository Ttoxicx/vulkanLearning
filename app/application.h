#pragma once

#include "base.h"
#include "vulkan_wrapper/instance.h"
#include "vulkan_wrapper/device.h"
#include "vulkan_wrapper/window.h"
#include "vulkan_wrapper/window_surface.h"
#include "vulkan_wrapper/swap_chain.h"
#include "vulkan_wrapper/shader.h"
#include "vulkan_wrapper/pipeline.h"
#include "vulkan_wrapper/render_pass.h"
#include "vulkan_wrapper/command_pool.h"
#include "vulkan_wrapper/command_buffer.h"
#include "vulkan_wrapper/semaphore.h"
#include "vulkan_wrapper/fence.h"
#include "vulkan_wrapper/buffer.h"
#include "vulkan_wrapper/descriptor_set_layout.h"
#include "vulkan_wrapper/descriptor_pool.h"
#include "vulkan_wrapper/descriptor_set.h"
#include "vulkan_wrapper/image.h"
#include "vulkan_wrapper/sampler.h"
#include "uniform_manager.h"
#include "texture/texture.h"


#include "model.h"
#include "camera.h"

namespace FF {

	class Application :public std::enable_shared_from_this<Application> {
	public:
		Application() = default;

		~Application() = default;

		void run();

		void onMouseMove(double xpos, double ypos);

		void onKeyDown(Camera::CAMERA_MOVE moveDirection);
	private:
		void initWindow();

		void initVulkan();

		void mainLoop();

		void render();

		void cleanUp();

	private:
		void createPipeline();

		void createRenderPass();

		void createCommandBuffers();

		void createSyncObjects();

		//重建交换链：当窗口大小发生变化的时候，交换链也要发生变化，Frame View Pipeline RenderPass Sync
		void reCreateSwapChain();

		void cleanUpSwapChain();

	private:
		unsigned int _width{ 800 };
		unsigned int _height{ 600 };

	private:
		Wrapper::Window::Ptr _window{ nullptr };
		Wrapper::Instance::Ptr _instance{ nullptr };
		Wrapper::Device::Ptr _device{ nullptr };
		Wrapper::WindowSurface::Ptr _surface{ nullptr };
		Wrapper::SwapChain::Ptr _swapChain{ nullptr };
		Wrapper::Pipeline::Ptr _pipeline{ nullptr };
		Wrapper::RenderPass::Ptr _renderPass{ nullptr };
		Wrapper::CommandPool::Ptr _commandPool{ nullptr };

		std::vector<Wrapper::CommandBuffer::Ptr> _commandBuffers{};

		int _currentFrame{ 0 };

		std::vector<Wrapper::Fence::Ptr> _fences{};
		std::vector<Wrapper::Semaphore::Ptr> _imageAvailableSemaphores{};
		std::vector<Wrapper::Semaphore::Ptr> _renderFinishedSemaphores{};

		UniformManager::Ptr _uniformManager{ nullptr };

		Model::Ptr _model{ nullptr };
		VPMatrices _vpMatrices;
		Camera _camera;
	};
}