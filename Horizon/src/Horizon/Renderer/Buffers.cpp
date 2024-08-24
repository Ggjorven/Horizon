#include "hzpch.h"
#include "Buffers.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Descriptors.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include "Horizon/Vulkan/VulkanBuffers.hpp"

namespace Hz
{

    ///////////////////////////////////////////////////////////
	// Specifications
    ///////////////////////////////////////////////////////////
	size_t DataTypeSize(DataType type)
	{
		switch (type)
		{
		case DataType::Float:    return 4;
		case DataType::Float2:   return 4 * 2;
		case DataType::Float3:   return 4 * 3;
		case DataType::Float4:   return 4 * 4;
		case DataType::Mat3:     return 4 * 3 * 3;
		case DataType::Mat4:     return 4 * 4 * 4;
		case DataType::Int:      return 4;
		case DataType::Int2:     return 4 * 2;
		case DataType::Int3:     return 4 * 3;
		case DataType::Int4:     return 4 * 4;
		case DataType::Bool:     return 1;

		default:
			HZ_ASSERT(false, "Unknown DataType!");
			break;
		}

		return 0;
	}

	BufferElement::BufferElement(DataType type, uint32_t location, const std::string& name)
		: Name(name), Location(location), Type(type), Size(DataTypeSize(type)), Offset(0)
	{
	}

	uint32_t BufferElement::GetComponentCount() const
	{
		switch (Type)
		{
		case DataType::Float:   return 1;
		case DataType::Float2:  return 2;
		case DataType::Float3:  return 3;
		case DataType::Float4:  return 4;
		case DataType::Mat3:    return 3 * 3;
		case DataType::Mat4:    return 4 * 4;
		case DataType::Int:     return 1;
		case DataType::Int2:    return 2;
		case DataType::Int3:    return 3;
		case DataType::Int4:    return 4;
		case DataType::Bool:    return 1;

		default:
			HZ_ASSERT(false, "Unknown DataType!");
			break;
		}

		return 0;
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
		: m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	void BufferLayout::CalculateOffsetsAndStride()
	{
		size_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}

    ///////////////////////////////////////////////////////////
    // Buffers
    ///////////////////////////////////////////////////////////
    void VertexBuffer::Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            VulkanVertexBuffer::Bind(commandBuffer, std::move(buffers));
    }

    Ref<VertexBuffer> VertexBuffer::Create(const BufferSpecification& specs, void* data, size_t size)
	{
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanVertexBuffer>::Create(specs, data, size);

		return nullptr;
	}

    Ref<IndexBuffer> IndexBuffer::Create(const BufferSpecification& specs, uint32_t* indices, uint32_t count)
    {
		if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanIndexBuffer>::Create(specs, indices, count);

		return nullptr;
	}

    Ref<UniformBuffer> UniformBuffer::Create(const BufferSpecification& specs, size_t dataSize)
    {
		if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanUniformBuffer>::Create(specs, dataSize);

		return nullptr;
	}

    /* // TODO: To be implemented
	Ref<DynamicUniformBuffer> DynamicUniformBuffer::Create(uint32_t elements, size_t sizeOfOneElement)
	{
		switch (RendererSpecification::API)
		{
		case RenderingAPI::Vulkan:
			return RefHelper::Create<VulkanDynamicUniformBuffer>(elements, sizeOfOneElement);

		default:
			APP_LOG_ERROR("Invalid API selected.");
			break;
		}

		return nullptr;
	}
    */

    Ref<StorageBuffer> StorageBuffer::Create(const BufferSpecification& specs, size_t dataSize)
    {
		if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanStorageBuffer>::Create(specs, dataSize);

		return nullptr;
	}

}
