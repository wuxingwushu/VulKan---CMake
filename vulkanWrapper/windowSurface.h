#pragma once

#include "../base.h"
#include "instance.h"
#include "window.h"

namespace FF::Wrapper {

	class WindowSurface {
	public:
		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, Window::Ptr window) {
			return std::make_shared<WindowSurface>(instance, window);
		}

		//创建Surface，让VulKan和窗口链接起来（适配win,安卓,等等不同设备）
		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		//获取Surface的指针
		[[nodiscard]] auto getSurface() const { return mSurface; }

	private:
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };
	};
}
