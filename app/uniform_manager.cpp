#include "uniform_manager.h"

UniformManager::UniformManager(){

}

UniformManager::~UniformManager(){

}

void UniformManager::init(const FF::Wrapper::Device::Ptr& device,const FF::Wrapper::CommandPool::Ptr& commandPool,int frameCount) {
	
	_device = device;

	//descriptor
	auto vpParam = FF::Wrapper::UniformParameter::create();
	vpParam->mBinding = 0;
	vpParam->mCount = 1;
	vpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpParam->mSize = sizeof(VPMatrices);
	vpParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

	for (int i = 0; i < frameCount; ++i) {
		auto buffer = FF::Wrapper::Buffer::createUniformBuffer(device, vpParam->mSize, nullptr);
		vpParam->mBuffers.push_back(buffer);
	}

	_uniformParams.push_back(vpParam);

	auto objectParam = FF::Wrapper::UniformParameter::create();
	objectParam->mBinding = 1;
	objectParam->mCount = 1;
	objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	objectParam->mSize = sizeof(ObjectUniform);
	objectParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

	for (int i = 0; i < frameCount; ++i) {
		auto buffer = FF::Wrapper::Buffer::createUniformBuffer(device, objectParam->mSize, nullptr);
		objectParam->mBuffers.push_back(buffer);
	}

	_uniformParams.push_back(objectParam);

	auto textureParam = FF::Wrapper::UniformParameter::create();
	textureParam->mBinding = 2;
	textureParam->mCount = 1;
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	textureParam->mTexture = FF::Texture::create(_device, commandPool, "assets/asuka_langley.jpg");

	_uniformParams.push_back(textureParam);

	_descriptorSetLayout = FF::Wrapper::DescriptorSetLayout::create(device);
	_descriptorSetLayout->build(_uniformParams);

	_descriptorPool = FF::Wrapper::DescriptorPool::create(device);
	_descriptorPool->build(_uniformParams, frameCount);

	_descrptorSet = FF::Wrapper::DescriptorSet::create(
		device, _uniformParams,
		_descriptorSetLayout,
		_descriptorPool, frameCount
	);
}

void UniformManager::update(const VPMatrices& vpMatrices, const ObjectUniform& objUnifom, int frameCount) {
	//update VP Matrices
	_uniformParams[0]->mBuffers[frameCount]->updateBufferByMap((void*)(&vpMatrices), sizeof(VPMatrices));
	//update Obj uniform
	_uniformParams[1]->mBuffers[frameCount]->updateBufferByMap((void*)(&objUnifom), sizeof(ObjectUniform));
}