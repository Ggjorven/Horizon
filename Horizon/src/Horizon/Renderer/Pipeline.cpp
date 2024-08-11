#include "hzpch.h"
#include "Pipeline.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanPipeline.hpp"

namespace Hz
{

    Pipeline::Pipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass)
        : m_Instance(new PipelineType(specs, sets, shader, renderpass))
    {
    }

    Pipeline::Pipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader)
        : m_Instance(new PipelineType(specs, sets, shader))
    {
    }

    Pipeline::~Pipeline()
    {
        delete m_Instance;
    }

    void Pipeline::Use(Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint)
    {
        m_Instance->Use(commandBuffer, bindPoint);
    }

    void Pipeline::DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth)
    {
        m_Instance->DispatchCompute(commandBuffer, width, height, depth);
    }

    const PipelineSpecification& Pipeline::GetSpecification() const
    {
        return m_Instance->GetSpecification();
    }

    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass)
    {
        return Ref<Pipeline>::Create(specs, sets, shader, renderpass);
    }

    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader)
    {
        return Ref<Pipeline>::Create(specs, sets, shader);
    }

}
