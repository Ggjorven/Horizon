#pragma once

#include "Horizon/Core/Memory.hpp"
#include "Horizon/Core/Logging.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT_HELPER_2(expr, exprStr, randomizedNr) \
VkResult result##randomizedNr = expr; \
if (result##randomizedNr != VK_SUCCESS) \
	HZ_LOG_ERROR("Expression {0} failed with error code: {1}", exprStr, Pulse::Enum::Name(result##randomizedNr))

#define VK_CHECK_RESULT_HELPER(expr, exprStr, randomizedNr) VK_CHECK_RESULT_HELPER_2(expr, exprStr, randomizedNr)
#define VK_CHECK_RESULT(expr) VK_CHECK_RESULT_HELPER((expr), #expr, __COUNTER__)

namespace Hz::VkUtils
{

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

}
