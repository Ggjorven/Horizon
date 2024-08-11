#pragma once

#include "Horizon/Core/Memory.hpp"
#include "Horizon/Core/Logging.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define VK_CHECK_RESULT_HELPER_2(expr, exprStr, randomizedNr) \
VkResult result##randomizedNr = expr; \
if (result##randomizedNr != VK_SUCCESS) \
	HZ_LOG_ERROR("Expression {0} failed with error code: {1}", exprStr, Pulse::Enum::Name(result##randomizedNr))

#define VK_CHECK_RESULT_HELPER(expr, exprStr, randomizedNr) VK_CHECK_RESULT_HELPER_2(expr, exprStr, randomizedNr)

#if !defined(HZ_CONFIG_DIST)
    #define VK_CHECK_RESULT(expr) VK_CHECK_RESULT_HELPER((expr), #expr, __COUNTER__)
#else
    #define VK_CHECK_RESULT(expr) expr
#endif

namespace Hz::VkUtils
{

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    // A vulkan specific helper class for allocating Buffers/Images using VMA
    class Allocator
    {
    public:
        static void Init();
        static void Destroy();

    public:
        // Buffers
        static VmaAllocation AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, VkMemoryPropertyFlags requiredFlags = 0);
		static void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
		static void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

        // Image
        static VmaAllocation AllocateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VkMemoryPropertyFlags requiredFlags = {});
		static void CopyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);
		static VkImageView CreateImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		static VkSampler CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels);
		static void DestroyImage(VkImage image, VmaAllocation allocation);

        // Utils
        static void MapMemory(VmaAllocation& allocation, void*& mapData);
		static void UnMapMemory(VmaAllocation& allocation);
    };

}
