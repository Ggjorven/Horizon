#include "hzpch.h"
#include "VulkanUtils.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

// Note: This file builds VMA
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace Hz::VkUtils
{

    static VmaAllocator s_Allocator = VK_NULL_HANDLE;

    ///////////////////////////////////////////////////////////
    // General helper functions
	///////////////////////////////////////////////////////////
    VkFormat FindDepthFormat()
	{
		return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		HZ_LOG_ERROR("Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

    //////////////////////////////////////////////////////////
    // Initialization functions
	///////////////////////////////////////////////////////////
    static void* VKAPI_PTR VmaAllocFn(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
	{
		return std::malloc(size);
	}

	static void VKAPI_PTR VmaFreeFn(void* pUserData, void* pMemory)
	{
		std::free(pMemory);
	}

	static void* VKAPI_PTR VmaReallocFn(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
	{
		return std::realloc(pOriginal, size);
	}

	void Allocator::Init()
	{
		VkAllocationCallbacks callbacks = {};
		callbacks.pUserData = nullptr;
		callbacks.pfnAllocation = VmaAllocFn;
		callbacks.pfnFree = VmaFreeFn;
		callbacks.pfnReallocation = VmaReallocFn;
		callbacks.pfnInternalAllocation = nullptr;
		callbacks.pfnInternalFree = nullptr;

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.instance = VulkanContext::GetVkInstance();
		allocatorInfo.physicalDevice = VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice();
		allocatorInfo.device = VulkanContext::GetDevice()->GetVkDevice();
		allocatorInfo.pAllocationCallbacks = &callbacks;

        VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &s_Allocator));
	}

    void Allocator::InitPipelineCache(const std::vector<uint8_t>& data)
    {
        VkPipelineCacheCreateInfo cacheCreateInfo = {};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        cacheCreateInfo.initialDataSize = data.size();
        cacheCreateInfo.pInitialData = data.data();

        VK_CHECK_RESULT(vkCreatePipelineCache(VulkanContext::GetDevice()->GetVkDevice(), &cacheCreateInfo, nullptr, &s_PipelineCache));
    }

    void Allocator::Destroy()
	{
        vmaDestroyAllocator(s_Allocator);
        s_Allocator = VK_NULL_HANDLE;
	}

    //////////////////////////////////////////////////////////
    // Helper allocation functions
	///////////////////////////////////////////////////////////
    // Buffers
    VmaAllocation Allocator::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, VkMemoryPropertyFlags requiredFlags)
    {
        VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Change if necessary

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage; // VMA_MEMORY_USAGE_GPU_ONLY, VMA_MEMORY_USAGE_CPU_ONLY, etc.
		allocInfo.requiredFlags = requiredFlags;

		VmaAllocation allocation = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vmaCreateBuffer(s_Allocator, &bufferInfo, &allocInfo, &dstBuffer, &allocation, nullptr));

		return allocation;
    }

    void Allocator::CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
    {
        VulkanCommand command = VulkanCommand(true);

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(command.GetVkCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

		command.EndAndSubmit();
    }

    void Allocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(s_Allocator, buffer, allocation);
    }

    // Image
    VmaAllocation Allocator::AllocateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VkMemoryPropertyFlags requiredFlags)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = memUsage;
		allocCreateInfo.requiredFlags = requiredFlags;

		VmaAllocation allocation = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vmaCreateImage(s_Allocator, &imageInfo, &allocCreateInfo, &image, &allocation, nullptr));

		return allocation;
	}

	void Allocator::CopyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height)
	{
		VulkanCommand command = VulkanCommand(true);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(command.GetVkCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		command.EndAndSubmit();
	}

	VkImageView Allocator::CreateImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.aspectMask = aspectFlags;

		VkImageView imageView = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateImageView(VulkanContext::GetDevice()->GetVkDevice(), &viewInfo, nullptr, &imageView));

		return imageView;
	}

	VkSampler Allocator::CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = addressmode;
		samplerInfo.addressModeV = addressmode;
		samplerInfo.addressModeW = addressmode;

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice(), &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;								// Can be disabled: just set VK_FALSE
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // And 1.0f

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = (float)mipLevels;
		samplerInfo.mipLodBias = 0.0f; // Optional

		VkSampler sampler = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateSampler(VulkanContext::GetDevice()->GetVkDevice(), &samplerInfo, nullptr, &sampler));

		return sampler;
	}

	void Allocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		vmaDestroyImage(s_Allocator, image, allocation);
	}

    // Utils
    void Allocator::MapMemory(VmaAllocation& allocation, void *&mapData)
    {
        vmaMapMemory(s_Allocator, allocation, &mapData);
    }

    void Allocator::UnMapMemory(VmaAllocation& allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }
}
