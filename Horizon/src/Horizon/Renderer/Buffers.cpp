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
		}

		HZ_ASSERT(false, "Unknown DataType!");
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
		}

		HZ_ASSERT(false, "Unknown DataType!");
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
    VertexBuffer::VertexBuffer(const BufferSpecification& specs, void *data, size_t size)
        : m_Instance(new VertexBufferType(specs, data, size))
    {
    }

    VertexBuffer::~VertexBuffer()
    {
        delete m_Instance;
    }

    void VertexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
    {
        m_Instance->Bind(commandBuffer);
    }

    void VertexBuffer::Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers)
    {
        VertexBufferType::Bind(commandBuffer, std::move(buffers));
    }

    Ref<VertexBuffer> VertexBuffer::Create(const BufferSpecification& specs, void* data, size_t size)
	{
		return Ref<VertexBuffer>::Create(specs, data, size);
	}

    IndexBuffer::IndexBuffer(const BufferSpecification& specs, uint32_t* indices, uint32_t count)
        : m_Instance(new IndexBufferType(specs, indices, count))
    {
    }

    IndexBuffer::~IndexBuffer()
    {
        delete m_Instance;
    }

    void IndexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
    {
        m_Instance->Bind(commandBuffer);
    }

    uint32_t IndexBuffer::GetCount() const
    {
        return m_Instance->GetCount();
    }

    Ref<IndexBuffer> IndexBuffer::Create(const BufferSpecification& specs, uint32_t* indices, uint32_t count)
    {
		return Ref<IndexBuffer>::Create(specs, indices, count);
	}

    UniformBuffer::UniformBuffer(const BufferSpecification& specs, size_t dataSize)
        : m_Instance(new UniformBufferType(specs, dataSize))
    {
    }

    UniformBuffer::~UniformBuffer()
    {
        delete m_Instance;
    }

    void UniformBuffer::SetData(void* data, size_t size, size_t offset)
    {
        m_Instance->SetData(data, size, offset);
    }

    size_t UniformBuffer::GetSize() const
    {
        return m_Instance->GetSize();
    }

    Ref<UniformBuffer> UniformBuffer::Create(const BufferSpecification& specs, size_t dataSize)
    {
		return Ref<UniformBuffer>::Create(specs, dataSize);
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

    StorageBuffer::StorageBuffer(const BufferSpecification& specs, size_t dataSize)
        : m_Instance(new StorageBufferType(specs, dataSize))
    {
    }

    StorageBuffer::~StorageBuffer()
    {
        delete m_Instance;
    }

    void StorageBuffer::SetData(void* data, size_t size, size_t offset)
    {
        m_Instance->SetData(data, size, offset);
    }

    size_t StorageBuffer::GetSize() const
    {
        return m_Instance->GetSize();
    }

    Ref<StorageBuffer> StorageBuffer::Create(const BufferSpecification& specs, size_t dataSize)
    {
		return Ref<StorageBuffer>::Create(specs, dataSize);
	}

}
