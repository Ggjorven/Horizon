#include "hzpch.h"
#include "Renderpass.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanRenderpass.hpp"

namespace Hz
{

    Ref<Renderpass> Renderpass::Create(const RenderpassSpecification& specs, Ref<CommandBuffer> commandBuffer)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanRenderpass>::Create(specs, commandBuffer);

        return nullptr;
    }

}
