#include "hzpch.h"
#include "Renderer.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanRenderer.hpp"

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

    uint32_t Renderer::GetCurrentFrame()
    {
        return s_Instance->GetCurrentFrame();
    }

    const RendererSpecification& Renderer::GetSpecification()
    {
        return s_Instance->GetSpecification();
    }
}
