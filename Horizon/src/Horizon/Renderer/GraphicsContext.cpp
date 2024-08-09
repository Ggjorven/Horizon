#include "hzpch.h"
#include "GraphicsContext.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz
{

    GraphicsContext::ContextType* GraphicsContext::s_Instance = nullptr;

    void GraphicsContext::Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight)
    {
        s_Instance = new ContextType(window);
        s_Instance->Init(width, height, vsync, framesInFlight);
    }

    void GraphicsContext::Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }

    std::vector<Ref<Image>> &GraphicsContext::GetSwapChainImages()
    {
        return s_Instance->GetSwapChainImages();
    }

    Ref<Image> GraphicsContext::GetDepthImage()
    {
        return s_Instance->GetDepthImage();
    }

}
