#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"

namespace FF::Wrapper {

	const std::vector<const char*> deviceRequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) { 
			return std::make_shared<Device>(instance, surface); 
		}

		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		~Device();

		//在所以设备中选择分数最高的
		void pickPhysicalDevice();

		//给设备评分
		int rateDevice(VkPhysicalDevice device);

		//判断设备是否符合要求
		bool isDeviceSuitable(VkPhysicalDevice device);

		//初始化队列族
		void initQueueFamilies(VkPhysicalDevice device);

		//获取逻辑设备，创建对应引用ID
		void createLogicalDevice();

		//判断设备是否完整
		bool isQueueFamilyComplete();

		VkSampleCountFlagBits getMaxUsableSampleCount();

		[[nodiscard]] auto getDevice() const { return mDevice; }
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }

		[[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
		[[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }

		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue; }
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }

	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };//获得的详细设备信息
		Instance::Ptr mInstance{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };

		//***  Vulkan 将诸如绘制指令、内存操作提交到VkQueue 中，进行异步执行。

		//存储当前渲染任务队列族的id
		std::optional<uint32_t> mGraphicQueueFamily;
		VkQueue	mGraphicQueue{ VK_NULL_HANDLE };

		std::optional<uint32_t> mPresentQueueFamily;
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		//逻辑设备
		VkDevice mDevice{ VK_NULL_HANDLE };
	};
}