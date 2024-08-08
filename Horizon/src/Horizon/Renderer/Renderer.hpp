#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <Pulse/Enum/Enum.hpp>

namespace Hz
{

    class GraphicsContext;
    class VulkanRenderer;

    enum class ExecutionPolicy : uint8_t
    {
        InOrder = 1 << 0,           // Execute commands sequentially, submits to waited on by next (WaitForPrevious) commandBuffer
        Parallel = 1 << 1,          // Execute commands in parallel but synchronized by the frame
        WaitForPrevious = 1 << 2,   // Wait for the completion of the previous (InOrder) command buffer
        NoWait = 1 << 3             // Do not wait for the previous (InOrder) command buffer
    };
    using namespace Pulse::Enum::Bitwise;

    class Renderer
    {
    public:
        using RendererType = VulkanRenderer;
    public:
        static void Init(const RendererSpecification& specs);
        static void Destroy();

        // Has to be manually called by user on window resize events
        static void Recreate(uint32_t width, uint32_t height, const bool vsync);

        static void BeginFrame();
        static void EndFrame();
        static void Present();

        static uint32_t GetCurrentFrame();
        static const RendererSpecification& GetSpecification();

        static RendererType* Raw() { return s_Instance;}

    private:
        static RendererType* s_Instance;

        friend class GraphicsContext;
    };

}
