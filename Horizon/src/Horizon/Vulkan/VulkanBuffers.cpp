#include "hzpch.h"
#include "VulkanBuffers.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/Descriptors.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanDescriptors.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    VkFormat DataTypeToVkFormat(DataType type)
	{
		switch (type)
		{
		case DataType::Float:   return VK_FORMAT_R32_SFLOAT;
		case DataType::Float2:  return VK_FORMAT_R32G32_SFLOAT;
		case DataType::Float3:  return VK_FORMAT_R32G32B32_SFLOAT;
		case DataType::Float4:  return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DataType::Int:     return VK_FORMAT_R32_SINT;
		case DataType::Int2:    return VK_FORMAT_R32G32_SINT;
		case DataType::Int3:    return VK_FORMAT_R32G32B32_SINT;
		case DataType::Int4:    return VK_FORMAT_R32G32B32A32_SINT;
        case DataType::UInt:    return VK_FORMAT_R32_UINT;
        case DataType::UInt2:   return VK_FORMAT_R32G32_UINT;
        case DataType::UInt3:   return VK_FORMAT_R32G32B32_UINT;
        case DataType::UInt4:   return VK_FORMAT_R32G32B32A32_UINT;
		case DataType::Bool:    return VK_FORMAT_R8_UINT;
		case DataType::Mat3:    return VK_FORMAT_R32G32B32_SFLOAT;		// Assuming Mat3 is represented as 3 vec3s
		case DataType::Mat4:    return VK_FORMAT_R32G32B32A32_SFLOAT;	// Assuming Mat4 is represented as 4 vec4s

        default:
            HZ_ASSERT(false, "Invalid DataType passed in.");
            break;
        }

		return VK_FORMAT_UNDEFINED;
	}

    VulkanVertexBuffer::VulkanVertexBuffer(const BufferSpecification& specs, void* data, size_t size)
        : m_BufferSize(size)
    {
        m_Allocation = VkUtils::Allocator::AllocateBuffer(m_BufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

        // Only set data, if the data is valid
        if (data != nullptr)
            SetData(data, size, 0);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        Renderer::Free([buffer = m_Buffer, allocation = m_Allocation]()
        {
            if (buffer != VK_NULL_HANDLE)
                VkUtils::Allocator::DestroyBuffer(buffer, allocation);
        });
    }

    void VulkanVertexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = commandBuffer.As<VulkanCommandBuffer>();

        VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(vkCmdBuf->GetVkCommandBuffer(Renderer::GetCurrentFrame()), 0, 1, &m_Buffer, offsets);
    }

    void VulkanVertexBuffer::Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = commandBuffer.As<VulkanCommandBuffer>();

        std::vector<VkBuffer> vkBuffers;
        vkBuffers.reserve(buffers.size());

        std::vector<VkDeviceSize> offsets(buffers.size(), 0);

        for (auto& buffer : buffers)
        {
            Ref<VulkanVertexBuffer> vkVertexBuffer = buffer.As<VulkanVertexBuffer>();
            vkBuffers.push_back(vkVertexBuffer->m_Buffer);
        }

        vkCmdBindVertexBuffers(vkCmdBuf->GetVkCommandBuffer(Renderer::GetCurrentFrame()), 0, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());
    }

    void VulkanVertexBuffer::SetData(void* data, size_t size, size_t offset)
    {
        // Ensure that the size + offset doesn't exceed bounds
        HZ_VERIFY((size + offset <= m_BufferSize), "Size and offset exceeds the buffer's bounds");

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
        stagingBufferAllocation = VkUtils::Allocator::AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

        void* mappedData = nullptr;
        VkUtils::Allocator::MapMemory(stagingBufferAllocation, mappedData);
        std::memcpy(mappedData, data, size);  // Copy only 'size' bytes
        VkUtils::Allocator::UnMapMemory(stagingBufferAllocation);

        // Copy data from the staging buffer to the vertex buffer at the specified offset
        VkUtils::Allocator::CopyBuffer(stagingBuffer, m_Buffer, size, offset); 
        VkUtils::Allocator::DestroyBuffer(stagingBuffer, stagingBufferAllocation);
    }

    VulkanIndexBuffer::VulkanIndexBuffer(const BufferSpecification& specs, uint32_t* indices, uint32_t count)
        : m_Count(count)
    {
		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		m_Allocation = VkUtils::Allocator::AllocateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

        SetData(indices, count, 0);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        Renderer::Free([buffer = m_Buffer, allocation = m_Allocation]()
        {
            if (buffer != VK_NULL_HANDLE)
                VkUtils::Allocator::DestroyBuffer(buffer, allocation);
        });
    }

    void VulkanIndexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = commandBuffer.As<VulkanCommandBuffer>();

		vkCmdBindIndexBuffer(vkCmdBuf->GetVkCommandBuffer(Renderer::GetCurrentFrame()), m_Buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanIndexBuffer::SetData(uint32_t* indices, uint32_t count, size_t countOffset)
    {
        // Ensure that the size + offset doesn't exceed bounds
        HZ_VERIFY((count <= m_Count), "Count and countOffset exceeds the buffer's bounds");

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
        VkDeviceSize size = count * sizeof(uint32_t);
        stagingBufferAllocation = VkUtils::Allocator::AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

        void* mappedData = nullptr;
        VkUtils::Allocator::MapMemory(stagingBufferAllocation, mappedData);
        std::memcpy(mappedData, indices, size);  // Copy only 'size' bytes
        VkUtils::Allocator::UnMapMemory(stagingBufferAllocation);

        // Copy data from the staging buffer to the vertex buffer at the specified offset
        VkUtils::Allocator::CopyBuffer(stagingBuffer, m_Buffer, size, (countOffset * sizeof(uint32_t)));
        VkUtils::Allocator::DestroyBuffer(stagingBuffer, stagingBufferAllocation);
    }

    VulkanUniformBuffer::VulkanUniformBuffer(const BufferSpecification& specs, size_t dataSize)
        : m_Size(dataSize)
    {
        #if defined(HZ_CONFIG_DEBUG)
        if (specs.Usage == BufferMemoryUsage::GPU)
        {
            HZ_LOG_WARN("Creating a UniformBuffer solely on the GPU. This means SetData() cannot be used. Was this intented? If not, use: CPUToGPU.");
        }
        #endif

        const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		m_Buffers.resize(framesInFlight);
		m_Allocations.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++)
			m_Allocations[i] = VkUtils::Allocator::AllocateBuffer((VkDeviceSize)dataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VmaMemoryUsage)specs.Usage, m_Buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        Renderer::Free([buffers = m_Buffers, allocations = m_Allocations]()
        {
            for (size_t i = 0; i < buffers.size(); i++)
            {
                if (buffers[i] != VK_NULL_HANDLE)
                    VkUtils::Allocator::DestroyBuffer(buffers[i], allocations[i]);
            }
        });
    }

    void VulkanUniformBuffer::SetData(void* data, size_t size, size_t offset)
    {
        HZ_ASSERT((size + offset <= m_Size), "Data exceeds buffer size.")

		const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		for (size_t i = 0; i < framesInFlight; i++)
		{
			void* mappedMemory = nullptr;
			VkUtils::Allocator::MapMemory(m_Allocations[i], mappedMemory);
			memcpy(static_cast<uint8_t*>(mappedMemory) + offset, data, size);
			VkUtils::Allocator::UnMapMemory(m_Allocations[i]);
		}
    }

    VulkanStorageBuffer::VulkanStorageBuffer(const BufferSpecification& specs, size_t dataSize)
        : m_Size(dataSize)
    {
        #if defined(HZ_CONFIG_DEBUG)
        if (specs.Usage == BufferMemoryUsage::GPU)
        {
            HZ_LOG_WARN("Creating a StorageBuffer solely on the GPU. This means SetData() cannot be used. Was this intented? If not, use: CPUToGPU.");
        }
        #endif

        const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		m_Buffers.resize(framesInFlight);
		m_Allocations.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++)
			m_Allocations[i] = VkUtils::Allocator::AllocateBuffer((VkDeviceSize)dataSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, (VmaMemoryUsage)specs.Usage, m_Buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
	}

    VulkanStorageBuffer::~VulkanStorageBuffer()
    {
        Renderer::Free([buffers = m_Buffers, allocations = m_Allocations]()
        {
            for (size_t i = 0; i < buffers.size(); i++)
            {
                if (buffers[i] != VK_NULL_HANDLE)
                    VkUtils::Allocator::DestroyBuffer(buffers[i], allocations[i]);
            }
        });
    }

    void VulkanStorageBuffer::SetData(void* data, size_t size, size_t offset)
    {
        HZ_ASSERT((size + offset <= m_Size), "Data exceeds buffer size.");

		const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		for (size_t i = 0; i < framesInFlight; i++)
		{
			void* mappedMemory = nullptr;
			VkUtils::Allocator::MapMemory(m_Allocations[i], mappedMemory);
			memcpy(static_cast<uint8_t*>(mappedMemory) + offset, data, size);
			VkUtils::Allocator::UnMapMemory(m_Allocations[i]);
		}
    }

}
