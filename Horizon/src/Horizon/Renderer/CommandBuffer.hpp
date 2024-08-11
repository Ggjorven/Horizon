#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <type_traits>

namespace Hz
{

    class VulkanCommandBuffer;

    class CommandBuffer : public RefCounted
    {
    public:
        using CommandBufferType = VulkanCommandBuffer;
        static_assert(std::is_same_v<CommandBufferType, VulkanCommandBuffer>, "Unsupported command buffer type selected.");
    public:
        CommandBuffer();
        CommandBuffer(CommandBufferType* src);
        ~CommandBuffer();

        // Returns underlying type pointer
        inline CommandBufferType* Src() { return m_Instance; }

        static Ref<CommandBuffer> Create();

    private:
        CommandBufferType* m_Instance;
    };


}
