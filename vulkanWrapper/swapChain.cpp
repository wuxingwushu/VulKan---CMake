#include "swapChain.h"

namespace FF::Wrapper {

	SwapChain::SwapChain(
		const Device::Ptr& device, 
		const Window::Ptr& window, 
		const WindowSurface::Ptr& surface,
		const CommandPool::Ptr& commandPool
	) {
		mDevice = device;
		mWindow = window;
		mSurface = surface;

		auto swapChainSupportInfo = querySwapChainSupportInfo();

		//ѡ��vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

		//ѡ��prsentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

		//ѡ�񽻻�����Χ
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

		//����ͼ�񻺳�����
		mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;

		//���maxImageCountΪ0��˵��ֻҪ�ڴ治��ը�����ǾͿ����趨����������images
		if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount) {
			mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
		}

		//��д������Ϣ, �˴���ʼ�������ÿգ���Ϊ�����������õı�����ֵΪ���
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface->getSurface();
		createInfo.minImageCount = mImageCount;//���������õ��������ʺϵ�ǰ��������ǿ��ܻ�õ�����
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;

		//ͼ������Ĳ�Σ�VRһ���������
		createInfo.imageArrayLayers = 1;

		//���������ɵ�ͼ�񣬵������ںδ�
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		//��Ϊ��������ͼ�񣬻ᱻ������Ⱦ������ʾ������Ⱦ����ʾ�ֱ�ʹ�ò�ͬ�Ķ��У����Ի������������ʹ��ͬһ�������������
		//��ô���Ǿ���Ҫ���ã��ý�������ͼ�񣬱���������ʹ�ü���

		std::vector<uint32_t> queueFamilies = { mDevice->getGraphicQueueFamily().value() , mDevice->getPresentQueueFamily().value() };

		if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//��ĳһ���������ռ�����ܻ����
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//���Ա������ģʽ
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//��������ͼ���ʼ�仯�������Ƿ���Ҫ��ת
		createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;

		//����ԭ�����嵱�е����ݻ��
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//��ǰ���屻��ס�Ĳ��֣����û���,���ǻ�Ӱ�쵽�ض���
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create swapChain");
		}

		mSwapChainFormat = surfaceFormat.format;
		mSwapChainExtent = extent;

		//ϵͳ���ܴ��������image����ǰ��imageCount����С����
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
		mSwapChainImages.resize(mImageCount);

		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

		//����imageView
		mSwapChainImageViews.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i) {
			mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		//����depth image
		mDepthImages.resize(mImageCount);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		region.baseMipLevel = 0;
		region.levelCount = 1;
		region.baseArrayLayer = 0;
		region.layerCount = 1;

		for (int i = 0; i < mImageCount; ++i) {
			mDepthImages[i] = Image::createDepthImage(
				mDevice,
				mSwapChainExtent.width, 
				mSwapChainExtent.height,
				mDevice->getMaxUsableSampleCount()
			);
			
			mDepthImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				region,
				commandPool
			);
		}

		//����MutiSampleImages
		mMutiSampleImages.resize(mImageCount);

		VkImageSubresourceRange regionMutiSample{};
		regionMutiSample.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regionMutiSample.baseMipLevel = 0;
		regionMutiSample.levelCount = 1;
		regionMutiSample.baseArrayLayer = 0;
		regionMutiSample.layerCount = 1;

		for (int i = 0; i < mImageCount; ++i) {
			mMutiSampleImages[i] = Image::createRenderTargetImage(
				mDevice,
				mSwapChainExtent.width,
				mSwapChainExtent.height,
				mSwapChainFormat
			);

			mMutiSampleImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				regionMutiSample,
				commandPool
			);
		}
	}

	void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass) {
		//����FrameBuffer
		mSwapChainFrameBuffers.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i) {
			//FrameBuffer ����Ϊһ֡�����ݣ�������n��ColorAttachment 1��DepthStencilAttachment��
			//��Щ�����ļ���Ϊһ��FrameBuffer��������ߣ��ͻ��γ�һ��GPU�ļ��ϣ����Ϸ���Attachments����
			//ע�����鵱�е�˳�򣡣�������RenderPassƥ��
			std::array<VkImageView, 3> attachments = { 
				mSwapChainImageViews[i], 
				mMutiSampleImages[i]->getImageView(),
				mDepthImages[i]->getImageView() 
			};

			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = mSwapChainExtent.width;
			frameBufferCreateInfo.height = mSwapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error:Failed to create frameBuffer");
			}
		}
	}

	SwapChain::~SwapChain() {
		for (auto& imageView : mSwapChainImageViews) {
			vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
		}

		for (auto& frameBuffer : mSwapChainFrameBuffers) {
			vkDestroyFramebuffer(mDevice->getDevice(), frameBuffer, nullptr);
		}

		if (mSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
		}
		
		mWindow.reset();
		mSurface.reset();
		mDevice.reset();
	}

	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo info;
		//��ȡ��������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);

		//��ȡ����֧�ָ�ʽ
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			info.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());
		}

		//��ȡ����ģʽ
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			info.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());
		}

		return info;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		//���ֻ������һ��δ����ĸ�ʽ����ô��û����ѡ��ʽ�������Լ���һ��
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes) {
		//���豸�ϣ�ֻ��FIFO�Ǳ�����֧�ֵ�, ������ƶ��豸�ϣ�Ϊ�˽�ʡ��Դ������ѡ��FIFO
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresenstModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		//����������������˵��ϵͳ�����������Լ��趨extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		//���ڸ�����Ļ����� ������ƻ���� ����������С�������������صĳ���
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//�涨��max��min֮��
		actualExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view in swapchain");
		}

		return imageView;
	}
}