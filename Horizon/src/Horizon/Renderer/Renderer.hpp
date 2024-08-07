#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

namespace Hz
{

    class VulkanRenderer;

    class GraphicsContext;

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

        static const RendererSpecification& GetSpecification();

        static RendererType* Raw() { return s_Instance;}

    private:
        static RendererType* s_Instance;

        friend class GraphicsContext;
    };

}
