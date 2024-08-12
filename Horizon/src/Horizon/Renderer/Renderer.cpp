#include "hzpch.h"
#include "Renderer.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Buffers.hpp"

#include "Horizon/Vulkan/VulkanRenderer.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz
{

    Renderer::RendererType* Renderer::s_Instance = nullptr;

    void Renderer::Init(const RendererSpecification& specs)
    {
        s_Instance = new RendererType(specs);
    }

    void Renderer::Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }

    void Renderer::Recreate(uint32_t width, uint32_t height, const bool vsync)
    {
        s_Instance->Recreate(width, height, vsync);
    }

    void Renderer::BeginFrame()
    {
        s_Instance->BeginFrame();
    }

    void Renderer::EndFrame()
    {
        s_Instance->EndFrame();
    }

    void Renderer::Present()
    {
        s_Instance->Present();
    }

    void Renderer::BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state)
    {
        s_Instance->BeginDynamic(cmdBuf, std::move(state));
    }

    void Renderer::EndDynamic(Ref<CommandBuffer> cmdBuf)
    {
        s_Instance->EndDynamic(cmdBuf);
    }

    void Renderer::Begin(Ref<CommandBuffer> cmdBuf)
    {
        s_Instance->Begin(cmdBuf);
    }

    void Renderer::Begin(Ref<Renderpass> renderpass)
    {
        s_Instance->Begin(renderpass);
    }

    void Renderer::End(Ref<CommandBuffer> cmdBuf)
    {
        s_Instance->End(cmdBuf);
    }

    void Renderer::End(Ref<Renderpass> renderpass)
    {
        s_Instance->End(renderpass);
    }

    void Renderer::Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        s_Instance->Submit(cmdBuf, policy, queue, waitOn);
    }

    void Renderer::Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        s_Instance->Submit(renderpass, policy, queue, waitOn);
    }

    void Renderer::Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount)
    {
        s_Instance->Draw(cmdBuf, vertexCount, instanceCount);
    }

    void Renderer::DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount)
    {
        s_Instance->DrawIndexed(cmdBuf, indexBuffer, instanceCount);
    }

    // Note: The 2 functions below actually use the GraphicsContect since the queue needs to live even after the renderer is destroyed
    void Renderer::Free(FreeFunction&& func)
    {
        (*HzCast(VulkanContext, GraphicsContext::Src())).Free(std::move(func));
    }

    void Renderer::FreeObjects()
    {
        (*HzCast(VulkanContext, GraphicsContext::Src())).FreeObjects();
    }

    uint32_t Renderer::GetAcquiredImage()
    {
        return s_Instance->GetAcquiredImage();
    }

    uint32_t Renderer::GetCurrentFrame()
    {
        return s_Instance->GetCurrentFrame();
    }

    const RendererSpecification& Renderer::GetSpecification()
    {
        return s_Instance->GetSpecification();
    }
}
