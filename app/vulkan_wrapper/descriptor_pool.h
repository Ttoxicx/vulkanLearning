#pragma once

#include "../base.h"
#include "device.h"
#include "descriptor.h"

namespace FF::Wrapper {

	class DescriptorPool {
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<DescriptorPool>(device);
		}

		DescriptorPool(const Device::Ptr& device);

		~DescriptorPool();

		//frameCount 指有多少帧并行绘制(如为交换链中的每一张图像都创建uniforms防止数据冲突，例如
		//当前一帧提交的时候其它帧正在绘制，uniformbuffer，可能正在被读取，然而cpu的下一个循环可能
		//对其数据进行修改（比如纹理加载）)
		void build(std::vector<UniformParameter::Ptr>& params, const int frameCount);

		[[nodiscard]] VkDescriptorPool getPool() const { return _pool; }
	private:
		VkDescriptorPool _pool{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
	};
}