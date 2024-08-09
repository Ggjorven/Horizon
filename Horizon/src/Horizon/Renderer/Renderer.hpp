#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"
#include "Horizon/Renderer/Renderpass.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <type_traits>

namespace Hz
{

    using namespace Pulse::Enum::Bitwise;

    class GraphicsContext;
    class VulkanRenderer;

    enum class ExecutionPolicy : uint8_t
    {
        InOrder = 1 << 0,           // Execute commands sequentially, submits to waited on by next (WaitForPrevious) commandBuffer
        Parallel = 1 << 1,          // Execute commands in parallel but synchronized by the frame
        WaitForPrevious = 1 << 2,   // Wait for the completion of the previous (InOrder) command buffer
        NoWait = 1 << 3             // Do not wait for the previous (InOrder) command buffer
    };
    enum class Queue    : uint8_t  { Graphics, Present, Compute };

    class Renderer
    {
    public:
        using RendererType = VulkanRenderer;
        static_assert(std::is_same_v<RendererType, VulkanRenderer>, "Unsupported renderer type selected.");
    public:
        static void Init(const RendererSpecification& specs);
        static void Destroy();

        // Has to be manually called by user on window resize events
        static void Recreate(uint32_t width, uint32_t height, const bool vsync);

        static void BeginFrame();
        static void EndFrame();
        static void Present();

        // Execution of Renderpasses/CommandBuffers
        static void Begin(Ref<CommandBuffer> cmdBuf);
        static void Begin(Ref<Renderpass> renderpass);
        static void End(Ref<CommandBuffer> cmdBuf);
        static void End(Ref<Renderpass> renderpass);
        static void Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy = ExecutionPolicy::InOrder | ExecutionPolicy::WaitForPrevious, Queue queue = Queue::Graphics, const std::vector<Ref<CommandBuffer>>& waitOn = {});
        static void Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy = ExecutionPolicy::InOrder | ExecutionPolicy::WaitForPrevious, Queue queue = Queue::Graphics, const std::vector<Ref<CommandBuffer>>& waitOn = {});

        static uint32_t GetCurrentFrame();
        static const RendererSpecification& GetSpecification();

        inline static RendererType* Src() { return s_Instance;}

    private:
        static RendererType* s_Instance;

        friend class GraphicsContext;
    };

}
