#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"
#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "uniformManager.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/sampler.h"
#include "texture/texture.h"
#include "Camera.h"

#include "model.h"

namespace FF {
	//1 ����UniformBuffer������,���嵽buffer������
	//2 ����modelר�õ�uniform parameters�Լ�ȫ�ֹ��õ�uniform parameter

	class Application:public std::enable_shared_from_this<Application> {
	public:
		Application() = default;

		~Application() = default;

		//�ܳ�ʼ��
		void run();

		void onMouseMove(double xpos, double ypos);

		void onKeyDown(CAMERA_MOVE moveDirection);

	private:
		//���ڵĳ�ʼ��
		void initWindow();

		//��ʼ��Vulkan
		void initVulkan();

		//��ѭ��main
		void mainLoop();

		//��Ⱦһ֡����
		void render();

		//������Դ
		void cleanUp();

	private:
		void createPipeline();
		void createRenderPass();
		void createCommandBuffers();
		void createSyncObjects();

		//�ؽ�������:  �����ڴ�С�����仯��ʱ�򣬽�����ҲҪ�����仯��Frame View Pipeline RenderPass Sync
		void recreateSwapChain();

		void cleanupSwapChain();

	private:
		unsigned int mWidth{ 1960 };
		unsigned int mHeight{ 1080 };

	private:
		int mCurrentFrame{ 0 };//��Ⱦ��ǰ����Ŀ��
		Wrapper::Window::Ptr mWindow{ nullptr };//��������
		Wrapper::Instance::Ptr mInstance{ nullptr };//ʵ�л���Ҫ��VulKan����
		Wrapper::Device::Ptr mDevice{ nullptr };//��ȡ���Ե�Ӳ���豸
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };//��ȡ����ʲôƽ̨���е��ò�ͬ��API�����磺Window��Android��
		Wrapper::SwapChain::Ptr mSwapChain{ nullptr };//����VulKan�Ĺ���ϸ��
		Wrapper::Pipeline::Ptr mPipeline{ nullptr };//������Ⱦ���̹ܵ�
		Wrapper::RenderPass::Ptr mRenderPass{ nullptr };//�ռ�����Ҫ��Ⱦ��ͳһ��Ⱦ *
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };//ÿһ����Ⱦ��������� *

		std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};

		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};//ͼƬ�����ź���
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};//��Ⱦ����ź���
		std::vector<Wrapper::Fence::Ptr> mFences{};//���ƹ��߹��������磨��һ��������Ҫ��һ�����ߵ�ͼƬ���Ǿ͵���һ������ͼƬ��������ſ�ʼ������

		UniformManager::Ptr mUniformManager{ nullptr };

		Model::Ptr	mModel{ nullptr };
		VPMatrices	mVPMatrices;
		Camera      mCamera;
	};
}