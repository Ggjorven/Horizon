#include "hzpch.h"
#include "VulkanUtils.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz::VkUtils
{

    VkFormat FindDepthFormat()
	{
		return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
        const VulkanContext& context = GetHzContext(Vulkan);

		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(context.GetPhysicalDevice()->GetVkPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		HZ_LOG_ERROR("Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

}
