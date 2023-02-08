#pragma once

#include "../base.h"
#include "device.h"
#include "commandPool.h"

namespace FF::Wrapper {
	/*
	* 分析：如果我们需要做一张被用于纹理采样的图片，那么我们首先
	* 需要从undefinedLayout变换成为TransferDst， 然后在数据拷贝
	* 完毕之后，再转换称为ShaderReadOnly
	*/
	
	class Image {
	public:
		using Ptr = std::shared_ptr<Image>;

		//工具函数,静态
		static Image::Ptr createDepthImage(
			const Device::Ptr& device, 
			const int& width,
			const int& height,
			VkSampleCountFlagBits sample
		);

		static Image::Ptr createRenderTargetImage(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			VkFormat format
		);

	public:
		
		static Ptr create(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& tiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& sample,
			const VkMemoryPropertyFlags& properties,//memory
			const VkImageAspectFlags& aspectFlags//view
		) { 
			return std::make_shared<Image>(
				device,
				width,
				height,
				format,
				imageType,
				tiling,
				usage,
				sample,
				properties,
				aspectFlags
				); 
		}

		Image(
			const Device::Ptr &device,
			const int& width,
			const int& height,
			const VkFormat &format,
			const VkImageType &imageType,
			const VkImageTiling &tiling,
			const VkImageUsageFlags &usage,
			const VkSampleCountFlagBits &sample,
			const VkMemoryPropertyFlags &properties,//memory
			const VkImageAspectFlags &aspectFlags//view
		);

		~Image();

		//此处属于便捷写法，封装性比较好，但是可以独立作为一个工具函数
		//写到Tool的类里面
		void setImageLayout(
			VkImageLayout newLayout, 
			VkPipelineStageFlags srcStageMask, 
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresrouceRange,
			const CommandPool::Ptr & commandPool
			);

		void fillImageData(size_t size, void* pData, const CommandPool::Ptr &commandPool);

		[[nodiscard]] auto getImage() const { return mImage; }

		[[nodiscard]] auto getLayout() const { return mLayout; }

		[[nodiscard]] auto getWidth() const { return mWidth; }

		[[nodiscard]] auto getHeight() const { return mHeight; }

		[[nodiscard]] auto getImageView() const { return mImageView; }

	public:

		static VkFormat findDepthFormat(const Device::Ptr &device);
		static VkFormat findSupportedFormat(
			const Device::Ptr& device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);

		bool hasStencilComponent(VkFormat format);

	private:
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		

	private:
		size_t				mWidth{ 0 };
		size_t				mHeight{ 0 };
		Device::Ptr			mDevice{ nullptr };
		VkImage				mImage{ VK_NULL_HANDLE };
		VkDeviceMemory		mImageMemory{ VK_NULL_HANDLE };
		VkImageView			mImageView{ VK_NULL_HANDLE };
		VkFormat			mFormat;

		VkImageLayout		mLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	};
}