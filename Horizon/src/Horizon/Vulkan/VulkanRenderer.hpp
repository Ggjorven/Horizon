#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

#include "Horizon/Renderer/Renderer.hpp"

#include "Horizon/Vulkan/VulkanTaskManager.hpp"

#include <queue>
#include <mutex>

namespace Hz
{

    class VulkanSwapChain;

    class VulkanRenderer
    {
    public:
        static void Init(const RendererSpecification& specs);
        static bool Initialized();
        static void Destroy();

        static void Recreate(uint32_t width, uint32_t height, const bool vsync);

        static void BeginFrame();
        static void EndFrame();
        static void Present();

        static void BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state);
        static void EndDynamic(Ref<CommandBuffer> cmdBuf);

        static void Begin(Ref<CommandBuffer> cmdBuf);
        static void Begin(Ref<Renderpass> renderpass);
        static void End(Ref<CommandBuffer> cmdBuf);
        static void End(Ref<Renderpass> renderpass);
        static void Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn);
        static void Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn);

        static void Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount);
        static void DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount);

        static void Free(FreeFunction&& func);
        static void FreeObjects();

        static uint32_t GetAcquiredImage();
        static uint32_t GetCurrentFrame();

        inline static VulkanTaskManager& GetTaskManager() { return s_Data->Manager; }
        inline static const RendererSpecification& GetSpecification() { return s_Data->Specification; }

    private:
        static void VerifyExectionPolicy(ExecutionPolicy& policy);

    private:
        // Note: We store our info in a struct, so we can ensure lifetime
        // of all objects easily while the class remains static.
        struct Info
        {
        public:
            RendererSpecification Specification = {};

            VulkanTaskManager Manager = {};
        };

        inline static Info* s_Data = nullptr;

        // Note: We want to keep these alive till the end of the project,
        // because after Renderer::Destroy is called we still destroy some vulkan
        // related objects.
        inline static std::mutex s_FreeQueueMutex = {};
        inline static std::queue<FreeFunction> s_FreeQueue = {};

        friend class VulkanSwapChain;
    };

}
