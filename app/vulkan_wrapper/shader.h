#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	class Shader {
	public:
		using Ptr = std::shared_ptr<Shader>;
		static Ptr create(
			const Device::Ptr& device, 
			const std::string& fileName, 
			VkShaderStageFlagBits shaderStage, 
			const std::string& entryPoint
		) { 
			return std::make_shared<Shader>(device, fileName, shaderStage, entryPoint);
		}

		Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);
		~Shader();

		[[nodiscard]] VkShaderStageFlagBits getShaderStage() const { return _shaderStage; }
		[[nodiscard]] const std::string& getShaderEntryPoint() const { return _entryPoint; }
		[[nodiscard]] VkShaderModule getShaderModule() const { return _shaderModule; }
	private:
		VkShaderModule _shaderModule{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
		std::string _entryPoint;
		VkShaderStageFlagBits _shaderStage;
	};
}