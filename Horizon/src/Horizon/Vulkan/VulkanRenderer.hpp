#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

#include "Horizon/Renderer/Renderer.hpp"

#include "Horizon/Vulkan/VulkanTaskManager.hpp"

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

        uint32_t GetCurrentFrame() const;
        inline VulkanTaskManager& GetTaskManager() { return m_Manager; }
        inline const RendererSpecification& GetSpecification() const { return m_Specification; }

    private:
        void VerifyExectionPolicy(ExecutionPolicy& policy);

    private:
        RendererSpecification m_Specification;

        VulkanTaskManager m_Manager = {};

        friend class VulkanSwapChain;
    };

}
