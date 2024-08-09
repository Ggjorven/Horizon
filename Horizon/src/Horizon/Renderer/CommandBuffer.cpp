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

    CommandBuffer::CommandBuffer(CommandBufferType *src)
        : m_Instance(src)
    {
    }

    CommandBuffer::~CommandBuffer()
    {
        delete m_Instance;
    }

    Ref<CommandBuffer> CommandBuffer::Create()
    {
        return Ref<CommandBuffer>::Create();
    }

}
