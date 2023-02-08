#include "window.h"
#include "../application.h"
#include "../Camera.h"

namespace FF::Wrapper {
	
	static void windowResized(GLFWwindow* window, int width, int height) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->mWindowResized = true;
	}

	static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		auto app = pUserData->mApp;
		if (!app.expired()) {
			auto appReal = app.lock();
			appReal->onMouseMove(xpos, ypos);
		}
	}

	Window::Window(const int& width, const int& height) {
		mWidth = width;
		mHeight = height;

		glfwInit();

		//设置环境，关掉opengl API 并且禁止窗口改变大小
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//关掉opengl API
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);//是否禁止窗口改变大小

		mWindow = glfwCreateWindow(mWidth, mHeight, "Game_Demo - VulKan", nullptr, nullptr);//创建一个窗口
		if (!mWindow) {//判断窗口是否创建成功
			std::cerr << "Error: failed to create window" << std::endl;
		}

		glfwSetWindowUserPointer(mWindow, this);
		glfwSetFramebufferSizeCallback(mWindow, windowResized);
		glfwSetCursorPosCallback(mWindow, cursorPosCallBack);

		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	Window::~Window() {
		glfwDestroyWindow(mWindow);//回收GLFW的API
		glfwTerminate();
	}

	//判断窗口是否被关闭
	bool Window::shouldClose() {
		return glfwWindowShouldClose(mWindow);
	}

	//窗口获取事件
	void Window::pollEvents() {
		glfwPollEvents();
	}

	void Window::processEvent() {
		if (mApp.expired()) {
			return;
		}

		auto app = mApp.lock();

		if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			exit(0);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_FRONT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_BACK);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_LEFT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_RIGHT);
		}
	}
}