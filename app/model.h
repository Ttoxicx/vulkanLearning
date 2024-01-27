#pragma once

#include "base.h"
#include "vulkan_wrapper/buffer.h"
#include "vulkan_wrapper/device.h"

namespace FF {

	struct Vertex {
		glm::vec3 mPosition;
		glm::vec3 mColor;
		glm::vec3 mUV;
	};

	class Model {
	public:
		using Ptr = std::shared_ptr<Model>;
		static Ptr create(const Wrapper::Device::Ptr& device) { 
			return std::make_shared<Model>(device);
		}

		Model(const Wrapper::Device::Ptr& device) {

			/*mData = {
				{{0.0f,-0.5f,0.0f},{1.0f,0.0f,0.0f}},
				{{0.5f,0.5f,0.0f},{0.0f,1.0f,0.0f}},
				{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f}},
			};*/

			mPositions = {
				{0.0f,0.5f,0.0f},
				{0.5f,0.0f,0.0f},
				{0.0f,-0.5f,0.0f},
				{-0.5f,0.0f,0.0f},

				{-0.5f,0.5f,0.2f},
				{0.5f,0.5f,0.2f},
				{0.5f,-0.5f,0.2f},
				{-0.5f,-0.5f,0.2f}
			};

			mColors = {
				{1.0f,0.0f,0.0f},
				{0.0f,1.0f,0.0f},
				{0.0f,0.0f,1.0f},
				{1.0f,0.0f,0.0f},

				{1.0f,0.0f,0.0f},
				{0.0f,1.0f,0.0f},
				{0.0f,0.0f,1.0f},
				{1.0f,0.0f,0.0f}
			};

			mIndexData = {
				0,2,1,0,3,2,
				4,6,5,4,7,6
			};

			mUVs = {
				{0.0f,1.0f},
				{0.0f,0.0f},
				{1.0f,0.0f},
				{1.0f,1.0f},

				{0.0f,1.0f},
				{0.0f,0.0f},
				{1.0f,0.0f},
				{1.0f,1.0f}
			};

			//mVertexBuffer = Wrapper::Buffer::createVertexBuffer(device, mData.size() * sizeof(Vertex), mData.data());
			mPositionBuffer = Wrapper::Buffer::createVertexBuffer(device, mPositions.size() * sizeof(glm::vec3), mPositions.data());
			mColorBuffer = Wrapper::Buffer::createVertexBuffer(device, mColors.size() * sizeof(glm::vec3), mColors.data());
			mIndexBuffer = Wrapper::Buffer::createIndexBuffer(device, mIndexData.size() * sizeof(float), mIndexData.data());
			mUVBuffer = Wrapper::Buffer::createVertexBuffer(device, mUVs.size() * sizeof(glm::vec2), mUVs.data());
		}
		~Model() {

		}

		//顶点数组buffer相关信息
		std::vector<VkVertexInputBindingDescription> getVertexInputBingdingDescription() {
			std::vector<VkVertexInputBindingDescription> bindingDes{};
			/*bindingDes.resize(1);
			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(Vertex);
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;*/

			bindingDes.resize(3);
			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(glm::vec3);
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindingDes[1].binding = 1;
			bindingDes[1].stride = sizeof(glm::vec3);
			bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindingDes[2].binding = 2;
			bindingDes[2].stride = sizeof(glm::vec2);
			bindingDes[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDes;
		}

		//attribute相关信息，与Vertex里面的location相关
		std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription() {
			std::vector<VkVertexInputAttributeDescription> attributeDes{};
			attributeDes.resize(3);
			/*attributeDes[0].binding = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[0].location = 0;
			attributeDes[0].offset = offsetof(Vertex, mPosition);
			attributeDes[1].binding = 0;
			attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[1].location = 1;
			attributeDes[1].offset = offsetof(Vertex, mColor);*/
			attributeDes[0].binding = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[0].location = 0;
			attributeDes[0].offset = 0;
			attributeDes[1].binding = 1;
			attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[1].location = 1;
			attributeDes[1].offset = 0;
			attributeDes[2].binding = 2;
			attributeDes[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDes[2].location = 2;
			attributeDes[2].offset = 0;
			return attributeDes;
		}

		void setModelMatrix(const glm::mat4 matrix) { mUniform.mModelMatrix = matrix; }

		void update() {
			glm::mat4 rotateMatrix = glm::mat4(1.0f);
			rotateMatrix = glm::rotate(rotateMatrix, float(glfwGetTime() / 3.14), glm::vec3(0.0f, 0.0f, 1.0f));
			mUniform.mModelMatrix = rotateMatrix;
		}

		//[[nodiscard]] Wrapper::Buffer::Ptr getVertexBuffer() const { return mVertexBuffer; }

		[[nodiscard]] std::vector<VkBuffer> getVertexBuffers() const { 
			return { mPositionBuffer->getBuffer(),mColorBuffer->getBuffer(),mUVBuffer->getBuffer() };
		}

		[[nodiscard]] Wrapper::Buffer::Ptr getIndexBuffer() const { return mIndexBuffer; }

		[[nodiscard]] Wrapper::Buffer::Ptr getUVBuffer() const { return mUVBuffer; }

		[[nodiscard]] size_t getIndexCount() const { return mIndexData.size(); }

		[[nodiscard]] const ObjectUniform& getUniform() const { return mUniform; }

	private:
		//std::vector<Vertex> mData{};
		std::vector<glm::vec3> mPositions{};
		std::vector<glm::vec3> mColors{};
		std::vector<unsigned int> mIndexData{};
		std::vector<glm::vec2> mUVs{};

		//Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
		Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };
		Wrapper::Buffer::Ptr mColorBuffer{ nullptr };
		Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };
		Wrapper::Buffer::Ptr mUVBuffer{ nullptr };

		ObjectUniform mUniform;
	};
}