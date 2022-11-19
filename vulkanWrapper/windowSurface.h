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

		//����Surface����VulKan�ʹ�����������������win,��׿,�ȵȲ�ͬ�豸��
		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		//��ȡSurface��ָ��
		[[nodiscard]] auto getSurface() const { return mSurface; }

	private:
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };
	};
}
