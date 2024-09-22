#include "hzpch.h"
#include "Renderer.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Buffers.hpp"

#include "Horizon/Vulkan/VulkanRenderer.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

#include <Pulse/Enum/Enum.hpp>

namespace Hz
{

    // Static type selection
    template<RenderingAPI API> struct RendererSelector;
    template<> struct RendererSelector<RenderingAPI::Vulkan> { using Type = VulkanRenderer; };

    using RendererType = typename RendererSelector<RendererSpecification::API>::Type;

    void Renderer::Init(const RendererSpecification& specs)
    {
        RendererType::Init(specs);
    }

    bool Renderer::Initialized()
    {
        return RendererType::Initialized();;
    }

    void Renderer::Destroy()
    {
        RendererType::Destroy();
    }

    void Renderer::Recreate(uint32_t width, uint32_t height, const bool vsync)
    {
        RendererType::Recreate(width, height, vsync);
    }

    void Renderer::BeginFrame()
    {
        RendererType::BeginFrame();
    }

    void Renderer::EndFrame()
    {
        RendererType::EndFrame();
    }

    void Renderer::Present()
    {
        RendererType::Present();
    }

    void Renderer::BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state)
    {
        RendererType::BeginDynamic(cmdBuf, std::move(state));
    }

    void Renderer::EndDynamic(Ref<CommandBuffer> cmdBuf)
    {
        RendererType::EndDynamic(cmdBuf);
    }

    void Renderer::Begin(Ref<CommandBuffer> cmdBuf)
    {
        RendererType::Begin(cmdBuf);
    }

    void Renderer::Begin(Ref<Renderpass> renderpass)
    {
        RendererType::Begin(renderpass);
    }

    void Renderer::End(Ref<CommandBuffer> cmdBuf)
    {
        RendererType::End(cmdBuf);
    }

    void Renderer::End(Ref<Renderpass> renderpass)
    {
        RendererType::End(renderpass);
    }

    void Renderer::Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        RendererType::Submit(cmdBuf, policy, queue, waitStage, waitOn);
    }

    void Renderer::Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        RendererType::Submit(renderpass, policy, queue, waitStage, waitOn);
    }

    void Renderer::Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount)
    {
        RendererType::Draw(cmdBuf, vertexCount, instanceCount);
    }

    void Renderer::DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount)
    {
        RendererType::DrawIndexed(cmdBuf, indexBuffer, instanceCount);
    }

    // Note: The 2 functions below actually use the GraphicsContect since the queue needs to live even after the renderer is destroyed
    void Renderer::Free(FreeFunction&& func)
    {
        RendererType::Free(std::move(func));
    }

    void Renderer::FreeObjects()
    {
        RendererType::FreeObjects();
    }

    uint32_t Renderer::GetAcquiredImage()
    {
        return RendererType::GetAcquiredImage();
    }

    uint32_t Renderer::GetCurrentFrame()
    {
        return RendererType::GetCurrentFrame();
    }

    const RendererSpecification& Renderer::GetSpecification()
    {
        return RendererType::GetSpecification();
    }

}
