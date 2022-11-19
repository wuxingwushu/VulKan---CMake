#pragma once

#include "../base.h"

namespace FF {
	class Application;
}

namespace FF::Wrapper {

	class Window {
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) { 
			return std::make_shared<Window>(width, height); 
		}

		//构建窗口
		Window(const int &width, const int &height);

		//解析释放
		~Window();

		//判断窗口是否被关闭
		bool shouldClose();

		//窗口获取事件
		void pollEvents();

		[[nodiscard]] auto getWindow() const { return mWindow; }

		void setApp(std::shared_ptr<Application> app) { mApp = app; }

		void processEvent();

	public:
		bool mWindowResized{ false };
		std::weak_ptr<Application> mApp;

	private:
		int mWidth{ 0 };//储存窗口宽度
		int mHeight{ 0 };//储存窗口高度
		GLFWwindow* mWindow{ NULL };//储存窗口指针
	};
}
