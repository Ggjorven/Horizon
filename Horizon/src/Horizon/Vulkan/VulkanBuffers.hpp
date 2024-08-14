#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Buffers.hpp"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Hz
{

    class VulkanDescriptorSet;

    VkFormat DataTypeToVkFormat(DataType type);

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const BufferSpecification& specs, void* data, size_t size);
		~VulkanVertexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const override;
		static void Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers);

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		size_t m_BufferSize;
	};

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const BufferSpecification& specs, uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const override;

		inline uint32_t GetCount() const override { return m_Count; }

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		uint32_t m_Count;
	};

	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(const BufferSpecification& specs, size_t dataSize);
		~VulkanUniformBuffer();

		void SetData(void* data, size_t size, size_t offset) override;

        inline size_t GetSize() const override { return m_Size; }

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		size_t m_Size;

        friend class VulkanDescriptorSet;
	};

    /* // TODO: Implement
	class VulkanDynamicUniformBuffer
	{
	public:
		VulkanDynamicUniformBuffer(uint32_t elements, size_t sizeOfOneElement);
		virtual ~VulkanDynamicUniformBuffer();

		void SetData(void* data, size_t size) override;

		void SetDataIndexed(uint32_t index, void* data, size_t size) override;
		void UploadIndexedData() override;

		inline uint32_t GetAmountOfElements() const override { return m_ElementCount; }
		inline size_t GetAlignment() const override { return m_AlignmentOfOneElement; }

		void Upload(Ref<DescriptorSet> set, Descriptor element) override;
		void Upload(Ref<DescriptorSet> set, Descriptor element, size_t offset) override;

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		uint32_t m_ElementCount = 0;
		size_t m_SizeOfOneElement = 0;
		size_t m_AlignmentOfOneElement = 0;

		std::vector<std::pair<void*, size_t>> m_IndexedData = { };

        friend class VulkanDescriptorSet;
	};
    */

	class VulkanStorageBuffer : public StorageBuffer
    {
	public:
		VulkanStorageBuffer(const BufferSpecification& specs, size_t dataSize);
		~VulkanStorageBuffer();

		void SetData(void* data, size_t size, size_t offset) override;

		inline size_t GetSize() const override { return m_Size; }

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		size_t m_Size;

        friend class VulkanDescriptorSet;
	};

}
