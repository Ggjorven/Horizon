#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

namespace Hz
{

    class VulkanSwapChain;

    class VulkanRenderer
    {
    public:
        VulkanRenderer(const RendererSpecification& specs);
        ~VulkanRenderer();

        void Recreate(uint32_t width, uint32_t height, const bool vsync);

        void BeginFrame();
        void EndFrame();
        void Present();

        //void Begin();
        //void Begin();
        //void End();
        //void End();
        //void Submit();
        //void Submit();

        inline const RendererSpecification& GetSpecification() const { return m_Specification; }

    private:
        RendererSpecification m_Specification;

        friend class VulkanSwapChain;
    };

}
