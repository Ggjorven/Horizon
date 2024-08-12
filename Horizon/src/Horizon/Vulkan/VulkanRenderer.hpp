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

        void BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state);
        void EndDynamic(Ref<CommandBuffer> cmdBuf);

        void Begin(Ref<CommandBuffer> cmdBuf);
        void Begin(Ref<Renderpass> renderpass);
        void End(Ref<CommandBuffer> cmdBuf);
        void End(Ref<Renderpass> renderpass);
        void Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn);
        void Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn);

        void Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount);

        uint32_t GetAcquiredImage() const;
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
