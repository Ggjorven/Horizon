#include "hzpch.h"
#include "Renderpass.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanRenderpass.hpp"

namespace Hz
{

    Renderpass::Renderpass(const RenderpassSpecification& specs, Ref<CommandBuffer> commandBuffer)
        : m_Instance(new RenderpassType(specs, (commandBuffer ? commandBuffer : CommandBuffer::Create())))
    {
    }

    Renderpass::Renderpass(RenderpassType *src)
        : m_Instance(src)
    {
    }

    Renderpass::~Renderpass()
    {
        delete m_Instance;
    }

    void Renderpass::Resize(uint32_t width, uint32_t height)
    {
        m_Instance->Resize(width, height);
    }

    std::pair<uint32_t, uint32_t> Renderpass::GetSize() const
    {
        return m_Instance->GetSize();
    }

    const RenderpassSpecification &Renderpass::GetSpecification()
    {
        return m_Instance->GetSpecification();
    }

    Ref<CommandBuffer> Renderpass::GetCommandBuffer()
    {
        return m_Instance->GetCommandBuffer();
    }

    Ref<Renderpass> Renderpass::Create(const RenderpassSpecification &specs, Ref<CommandBuffer> commandBuffer)
    {
        return Ref<Renderpass>::Create(specs, commandBuffer);
    }

}
