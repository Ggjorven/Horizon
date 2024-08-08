#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

namespace Hz
{

    class VulkanCommandBuffer;

    class CommandBuffer
    {
    public:
        using CommandBufferType = VulkanCommandBuffer;
    public:
        CommandBuffer();
        ~CommandBuffer();

        CommandBufferType* Src() { return m_Instance; }

    private:
        CommandBufferType* m_Instance;
    };


}
