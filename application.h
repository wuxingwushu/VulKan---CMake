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
	//1 定义UniformBuffer描述符,定义到buffer类里面
	//2 区分model专用的uniform parameters以及全局公用的uniform parameter

	class Application:public std::enable_shared_from_this<Application> {
	public:
		Application() = default;

		~Application() = default;

		//总初始化
		void run();

		void onMouseMove(double xpos, double ypos);

		void onKeyDown(CAMERA_MOVE moveDirection);

	private:
		//窗口的初始化
		void initWindow();

		//初始化Vulkan
		void initVulkan();

		//主循环main
		void mainLoop();

		//渲染一帧画面
		void render();

		//回收资源
		void cleanUp();

	private:
		void createPipeline();
		void createRenderPass();
		void createCommandBuffers();
		void createSyncObjects();

		//重建交换链:  当窗口大小发生变化的时候，交换链也要发生变化，Frame View Pipeline RenderPass Sync
		void recreateSwapChain();

		void cleanupSwapChain();

	private:
		unsigned int mWidth{ 1960 };
		unsigned int mHeight{ 1080 };

	private:
		int mCurrentFrame{ 0 };//渲染当前画面的框架
		Wrapper::Window::Ptr mWindow{ nullptr };//创建窗口
		Wrapper::Instance::Ptr mInstance{ nullptr };//实列化需要的VulKan功能
		Wrapper::Device::Ptr mDevice{ nullptr };//获取电脑的硬件设备
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };//获取你在什么平台运行调用不同的API（比如：Window，Android）
		Wrapper::SwapChain::Ptr mSwapChain{ nullptr };//设置VulKan的工作细节
		Wrapper::Pipeline::Ptr mPipeline{ nullptr };//设置渲染流程管道
		Wrapper::RenderPass::Ptr mRenderPass{ nullptr };//收集所有要渲染的统一渲染 *
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };//每一个渲染物体的命令 *

		std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};

		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};//图片可用信号量
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};//渲染完成信号量
		std::vector<Wrapper::Fence::Ptr> mFences{};//控制管线工作，比如（下一个管线需要上一个管线的图片，那就等上一个管线图片输入进来才开始工作）

		UniformManager::Ptr mUniformManager{ nullptr };

		Model::Ptr	mModel{ nullptr };
		VPMatrices	mVPMatrices;
		Camera      mCamera;
	};
}