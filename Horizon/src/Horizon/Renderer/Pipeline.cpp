#include "hzpch.h"
#include "Pipeline.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanPipeline.hpp"

namespace Hz
{

    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanPipeline>::Create(specs, sets, shader, renderpass);

        return nullptr;
    }

    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanPipeline>::Create(specs, sets, shader);

        return nullptr;
    }

}
