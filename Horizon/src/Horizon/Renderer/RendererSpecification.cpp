#include "hzpch.h"
#include "RendererSpecification.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"

namespace Hz
{

    DeviceSpecification DeviceSpecification::Get()
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return VkUtils::GetDeviceSpecs();

        return {};
    }

}