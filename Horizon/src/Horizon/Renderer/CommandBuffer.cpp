#include "hzpch.h"
#include "CommandBuffer.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    Ref<CommandBuffer> CommandBuffer::Create()
    {
        return Ref<VulkanCommandBuffer>::Create();
    }

}
