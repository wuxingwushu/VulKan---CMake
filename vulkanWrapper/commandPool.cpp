#include "commandPool.h"

namespace FF::Wrapper {

	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		mDevice = device;

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();

		//ָ���޸ĵ����ԡ�ָ��ص��ڴ�����
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: ���������CommandBuffer���Ե������¡���������
		//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: ÿһ��CommandBuffer���ܵ���reset�����뼯�����ã�vkResetCommandPool
		createInfo.flags = flag;

		if (vkCreateCommandPool(mDevice->getDevice(), &createInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error:  failed to create command pool");
		}
	}

	CommandPool::~CommandPool() {
		if (mCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
		}
	}
}