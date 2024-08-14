#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Image.hpp"

namespace Hz
{

    class GraphicsContext
    {
    public:
        static void Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);
        static void Destroy();

        static std::vector<Ref<Image>>& GetSwapChainImages();
		static Ref<Image> GetDepthImage();
    };

}
