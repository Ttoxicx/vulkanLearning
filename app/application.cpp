#include "application.h"
#include "vulkan_wrapper/image.h"

namespace FF {

	void Application::run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		_camera.onMouseMove(xpos, ypos);
	}

	void Application::onKeyDown(Camera::CAMERA_MOVE moveDirection) {
		_camera.move(moveDirection);
	}

	void Application::initWindow() {
		_window = Wrapper::Window::create(_width, _height);
		_window->setApp(shared_from_this());

		_camera.lookAt(
			glm::vec3(0.0f, 0.0f, 3.0f), 
			glm::vec3(0.0f, 0.0f, -1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		_camera.setPersipective(45.0f, (float)_width / float(_height), 0.1f, 100.0f);

		_camera.setSpeed(0.01f);
	}

	void Application::initVulkan() {
		_instance = Wrapper::Instance::create(true);
		_surface = Wrapper::WindowSurface::create(_instance, _window);
		_device = Wrapper::Device::create(_instance, _surface);
		_commandPool = Wrapper::CommandPool::create(_device);
		_swapChain = Wrapper::SwapChain::create(_device, _commandPool, _window, _surface);

		_width = _swapChain->getExtent().width;
		_height = _swapChain->getExtent().height;

		_renderPass = Wrapper::RenderPass::create(_device);
		createRenderPass();
		_swapChain->createFrameBuffers(_renderPass);

		//uniformManager
		_uniformManager = UniformManager::create();
		_uniformManager->init(_device, _commandPool, _swapChain->getImageCount());

		//����ģ��
		_model = Model::create(_device);


		_pipeline = Wrapper::Pipeline::create(_device, _renderPass);
		createPipeline();
		createCommandBuffers();
		createSyncObjects();
	}

	void Application::mainLoop() {
		while (!_window->shouldClose()) {
			_window->pollEvents();
			_window->proccessEvent();
			_model->update();
			_vpMatrices.mViewMatrix = _camera.getViewMatrix();
			_vpMatrices.mProjectionMatrix = _camera.getProjectionMatrix();
			_uniformManager->update(_vpMatrices, _model->getUniform(), _currentFrame);

			render();
		}
		vkDeviceWaitIdle(_device->getDevice());
	}

	void Application::render() {
		//�ȴ���ǰҪ�ύ��CommandBufferִ�����
		_fences[_currentFrame]->block();
		//��ȡ�������е���һ֡
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(
			_device->getDevice(),
			_swapChain->getSwapChain(),
			UINT64_MAX,
			_imageAvailableSemaphores[_currentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			reCreateSwapChain();
			_window->mWindowResized = false;
		}//VK_SUBOPTIMAL_KHR�õ�һ����Ϊ���õ�ͼ�񣬵������ʽ��һ��ƥ��
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: Failed to acquire next image");
		}

		//�����ύ��Ϣ
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//ͬ����Ϣ����Ⱦ������ʾͼ�����������ʾ��Ϻ󣬲��������ɫ
		VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//ָ���ύ��Щ����
		auto commandBuffer = _commandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		
		//����ִ�����֮��Ҫ������Щsemaphore
		VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		_fences[_currentFrame]->resetFence();
		if (vkQueueSubmit(_device->getGraphicQueue(), 1, &submitInfo, _fences[_currentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to submit render command");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { _swapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(_device->getPresentQueue(), &presentInfo);

		//������������һ����׼���������ǻ���Ҫ���Լ��ı�־λ�ж�
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _window->mWindowResized) {
			reCreateSwapChain();
			_window->mWindowResized = false;
		}
		else if( result != VK_SUCCESS ) {
			throw std::runtime_error("Error: failed to present");
		}

		_currentFrame = (_currentFrame + 1) % _swapChain->getImageCount();
	}

	void Application::cleanUp() {
		
	}

	void Application::createPipeline() {

		//�����ӿ�
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)_height;
		viewport.width = (float)_width;
		viewport.height = -(float)_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { _width,_height };

		_pipeline->setViewports({ viewport });
		_pipeline->setScissors({ scissor });

		//����shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		auto shaderVertex = Wrapper::Shader::create(_device, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		auto shaderFragment = Wrapper::Shader::create(_device, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderVertex);
		shaderGroup.push_back(shaderFragment);
		_pipeline->setShaderGroup(shaderGroup);

		//������Ų�ģʽ
		auto vertexBindingDes = _model->getVertexInputBingdingDescription();
		auto vertexAttribuDes = _model->getVertexInputAttributeDescription();
		_pipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDes.size());
		_pipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		_pipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribuDes.size());
		_pipeline->mVertexInputState.pVertexAttributeDescriptions = vertexAttribuDes.data();

		//ͼԪװ��
		_pipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		_pipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		_pipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		//��դ������
		_pipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		_pipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;//����ģʽ��Ҫ����GPU����
		_pipeline->mRasterState.lineWidth = 1.0f;//����1��Ҫ����GPU����
		_pipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		_pipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		_pipeline->mRasterState.depthBiasEnable = VK_FALSE;//
		_pipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		_pipeline->mRasterState.depthBiasClamp = 0.0f;
		_pipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		//���ز���
		_pipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		_pipeline->mSampleState.rasterizationSamples = _device->getMaxUsableSampleCount();
		_pipeline->mSampleState.minSampleShading = 1.0f;
		_pipeline->mSampleState.pSampleMask = nullptr;
		_pipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
		_pipeline->mSampleState.alphaToOneEnable = VK_FALSE;

		//�����ģ�����
		_pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		_pipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
		_pipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		//��ɫ���
		//�������ɫ������룬�õ��Ļ�Ͻ��������ͨ�����������AND���������
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		//��framebuffer˳��һ��
		_pipeline->pushBlendAttachment(blendAttachment);

		//1 blend�����ּ��㷽ʽ����һ�֣�����alphaΪ�����ļ��㣬�ڶ��ֽ���λ����
		//2 ���������logicOp����ô�Ϸ����õ�alphaΪ�����ļ��㣬ʧ��
		//3 ColorWrite���룬��Ȼ��Ч�����㿪����logicOP
		//4 ��Ϊ�����ǿ��ܻ��ж��FrameBuffer��������Կ�����Ҫ���BlendAttachment
		_pipeline->mBlendState.logicOpEnable = VK_FALSE;
		_pipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;

		//���blendAttachment��factor��operation
		_pipeline->mBlendState.blendConstants[0] = 0.0f;
		_pipeline->mBlendState.blendConstants[1] = 0.0f;
		_pipeline->mBlendState.blendConstants[2] = 0.0f;
		_pipeline->mBlendState.blendConstants[3] = 0.0f;

		//uniform�Ĵ���
		auto layout = _uniformManager->getDescriptorSetLayout()->getLayout();
		_pipeline->mLayoutCreateInfo.setLayoutCount = 1;
		_pipeline->mLayoutCreateInfo.pSetLayouts = &layout;
		_pipeline->mLayoutCreateInfo.pushConstantRangeCount = 0;
		_pipeline->mLayoutCreateInfo.pPushConstantRanges = nullptr;

		_pipeline->build();
	}

	void Application::createRenderPass() {
		//���뻭��������
		VkAttachmentDescription colorAttachmentDes{};
		colorAttachmentDes.format = _swapChain->getFormat();
		colorAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		_renderPass->addAttachment(colorAttachmentDes);

		VkAttachmentDescription multiSampleAttachmentDes{};
		multiSampleAttachmentDes.format = _swapChain->getFormat();
		multiSampleAttachmentDes.samples = _device->getMaxUsableSampleCount();
		multiSampleAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		multiSampleAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		multiSampleAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		multiSampleAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		multiSampleAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		multiSampleAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		_renderPass->addAttachment(multiSampleAttachmentDes);

		VkAttachmentDescription depthAttachmentDes{};
		depthAttachmentDes.format = Wrapper::Image::findDepthFormat(_device);
		depthAttachmentDes.samples = _device->getMaxUsableSampleCount();
		depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		_renderPass->addAttachment(depthAttachmentDes);

		//���ڵĻ��������������Լ�����Ҫ��
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference multiSampleAttachmentRef{};
		multiSampleAttachmentRef.attachment = 1;
		multiSampleAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//����������
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(multiSampleAttachmentRef);
		subPass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subPass.setResolvedAttachmentReference(colorAttachmentRef);
		subPass.buildSubPassDescription();
		_renderPass->addSubPass(subPass);

		//������֮���������ϵ
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		_renderPass->addDependency(dependency);

		_renderPass->buildPrenderPass();
	}

	void Application::createCommandBuffers() {
		_commandBuffers.resize(_swapChain->getImageCount());
		for (int i = 0; i < _swapChain->getImageCount(); ++i) {
			_commandBuffers[i] = Wrapper::CommandBuffer::create(_device, _commandPool);
			_commandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = _renderPass->getRenderPass();
			renderBeginInfo.framebuffer = _swapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0,0 };
			renderBeginInfo.renderArea.extent = _swapChain->getExtent();

			std::vector<VkClearValue> clearColors{};
			//�������չʾ��ɫ
			VkClearValue clearColor = {};
			clearColor.color = { 0.0f,0.0f,0.0f,1.0f };
			clearColors.push_back(clearColor);
			//������ز���
			VkClearValue clearMultiSampleColor = {};
			clearMultiSampleColor.color = { 0.0f,0.0f,0.0f,1.0f };
			clearColors.push_back(clearMultiSampleColor);
			//�������
			VkClearValue clearDepth{};
			clearDepth.depthStencil = { 1.0f,0 };
			clearColors.push_back(clearDepth);
			renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderBeginInfo.pClearValues = clearColors.data();

			_commandBuffers[i]->beginRenderPass(renderBeginInfo);
			_commandBuffers[i]->bindGraphicPipeline(_pipeline->getPipeline());
			_commandBuffers[i]->bindDescriptorSet(_pipeline->getPipelineLayout(), _uniformManager->getDescriptorSet(_currentFrame));
			//_commandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffer()->getBuffer() });
			_commandBuffers[i]->bindVertexBuffer(_model->getVertexBuffers());
			_commandBuffers[i]->bindIndexBuffer(_model->getIndexBuffer()->getBuffer());
			_commandBuffers[i]->drawIndex(_model->getIndexCount());
			_commandBuffers[i]->endRenderPass();
			_commandBuffers[i]->end();
		}
	}

	void Application::createSyncObjects() {
		for (int i = 0; i < _swapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(_device);
			_imageAvailableSemaphores.push_back(imageSemaphore);
			auto renderSemaphore = Wrapper::Semaphore::create(_device);
			_renderFinishedSemaphores.push_back(renderSemaphore);
			auto fence = Wrapper::Fence::create(_device);
			_fences.push_back(fence);
		}
	}

	void Application::reCreateSwapChain() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(_window->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(_window->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(_device->getDevice());
		cleanUpSwapChain();
		_swapChain = Wrapper::SwapChain::create(_device, _commandPool, _window, _surface);
		_width = _swapChain->getExtent().width;
		_height = _swapChain->getExtent().height;
		_renderPass = Wrapper::RenderPass::create(_device);
		createRenderPass();
		_swapChain->createFrameBuffers(_renderPass);
		_pipeline = Wrapper::Pipeline::create(_device, _renderPass);
		createPipeline();
		createCommandBuffers();
		createSyncObjects();
	}

	void Application::cleanUpSwapChain() {
		_swapChain.reset();
		_commandBuffers.clear();
		_pipeline.reset();
		_renderPass.reset();
		_imageAvailableSemaphores.clear();
		_renderFinishedSemaphores.clear();
		_fences.clear();
	}
}