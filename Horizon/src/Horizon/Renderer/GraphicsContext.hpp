#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Image.hpp"

#include <type_traits>

namespace Hz
{

    class VulkanContext;

    class Renderer;

    class GraphicsContext
    {
    public:
        using ContextType = VulkanContext;
        static_assert(std::is_same_v<ContextType, VulkanContext>, "Unsupported context type selected.");
    public:
        static void Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);
        static void Destroy();

        static std::vector<Ref<Image>>& GetSwapChainImages();
		static Ref<Image> GetDepthImage();

        inline static ContextType* Src() { return s_Instance;}

    private:
        static ContextType* s_Instance;

        friend class Renderer;
    };

    // A macro for avoiding type safe pointer casting while keeping it static
    // Don't include the * in the type.
    #define HzCast(type, obj) static_cast<type*>(static_cast<void*>(obj))

    // A helper macro to 'safely' retrieve the appropriate context type
    // Make sure that the type you're casting to is actually the type.
    #define GetHzContext(api) *(HzCast(api##Context, GraphicsContext::Src())) // TODO: rRemove

}
