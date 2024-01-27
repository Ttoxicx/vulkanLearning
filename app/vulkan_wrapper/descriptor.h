#pragma once

#include "buffer.h"
#include "../texture/texture.h"

namespace FF::Wrapper {
	//������Ҫ֪�������ֵ��У���������Щuniform,ÿһ��������binding,ÿһ��ʲô����
	struct UniformParameter {
		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() { return std::make_shared<UniformParameter>(); }
		size_t mSize{ 0 };
		uint32_t mBinding{ 0 };
		//����ÿһ��binding�㣬���п��ܴ������һ�����飬������һ��Matrix[],count�ʹ���������Ĵ�С��
		//��Ҫʹ��indexDescriptor����
		uint32_t mCount{ 0 };
		VkDescriptorType mDescriptorType;
		VkShaderStageFlagBits mStage;

		std::vector<Buffer::Ptr> mBuffers{};
		Texture::Ptr mTexture{ nullptr };
	};
}