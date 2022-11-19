#pragma once

#include "../base.h"

namespace FF::Wrapper {
	
	class Instance {
	public:
		using Ptr = std::shared_ptr<Instance>;
		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		Instance(bool enableValidationLayer);

		~Instance();

		std::vector<const char*> getRequiredExtensions();

		//layers
		bool checkValidationLayerSupport();
		void setupDebugger();

		[[nodiscard]] VkInstance getInstance() const { return mInstance; }

		//�ж��Ƿ����˼��
		[[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

	private:
		//��ӡ������չ��
		void printAvailableExtensions();

	private:
		VkInstance mInstance{ VK_NULL_HANDLE };
		bool mEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
	};
}