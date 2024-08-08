#include "hzpch.h"
#include "CommandBuffer.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    CommandBuffer::CommandBuffer()
        : m_Instance(new CommandBufferType())
    {
    }

    CommandBuffer::~CommandBuffer()
    {
        delete m_Instance;
    }

}
