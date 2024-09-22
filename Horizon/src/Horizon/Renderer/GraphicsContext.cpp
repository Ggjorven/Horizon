#include "hzpch.h"
#include "GraphicsContext.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz
{

    // Static type selection
    template<RenderingAPI API> struct GraphicsContextSelector;
    template<> struct GraphicsContextSelector<RenderingAPI::Vulkan> { using Type = VulkanContext; };

    using ContextType = typename GraphicsContextSelector<RendererSpecification::API>::Type;

    void GraphicsContext::Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight)
    {
        ContextType::Init(window, width, height, vsync, framesInFlight);
    }

    void GraphicsContext::Destroy()
    {
        ContextType::Destroy();
    }

    std::vector<Ref<Image>>& GraphicsContext::GetSwapChainImages()
    {
        return ContextType::GetSwapChainImages();
    }

    Ref<Image> GraphicsContext::GetDepthImage()
    {
        return ContextType::GetDepthImage();
    }

}
