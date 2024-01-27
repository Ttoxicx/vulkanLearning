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

		//frameCount ָ�ж���֡���л���(��Ϊ�������е�ÿһ��ͼ�񶼴���uniforms��ֹ���ݳ�ͻ������
		//��ǰһ֡�ύ��ʱ������֡���ڻ��ƣ�uniformbuffer���������ڱ���ȡ��Ȼ��cpu����һ��ѭ������
		//�������ݽ����޸ģ�����������أ�)
		void build(std::vector<UniformParameter::Ptr>& params, const int frameCount);

		[[nodiscard]] VkDescriptorPool getPool() const { return _pool; }
	private:
		VkDescriptorPool _pool{ VK_NULL_HANDLE };
		Device::Ptr _device{ nullptr };
	};
}