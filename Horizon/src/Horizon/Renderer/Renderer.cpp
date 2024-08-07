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

    const RendererSpecification &Renderer::GetSpecification()
    {
        return s_Instance->GetSpecification();
    }
}
