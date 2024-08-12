#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

namespace Hz
{

    class VulkanVertexBuffer;
    class VulkanIndexBuffer;
    class VulkanUniformBuffer;
    class VulkanDynamicUniformBuffer;
    class VulkanStorageBuffer;

    class CommandBuffer;

    ///////////////////////////////////////////////////////////
	// Specifications
    ///////////////////////////////////////////////////////////
	enum class DataType : uint8_t
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};
	size_t DataTypeSize(DataType type);

	struct BufferElement
	{
	public:
		std::string Name;
		uint32_t Location;
		DataType Type;
		uint32_t Size;
		size_t Offset;

    public:
		BufferElement() = default;
		BufferElement(DataType type, uint32_t location, const std::string& name);
		~BufferElement() = default;

		uint32_t GetComponentCount() const;
	};

	struct BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements);
		~BufferLayout() = default;

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		inline std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride();

	private:
		std::vector<BufferElement> m_Elements = { };
		uint32_t m_Stride = 0;
	};

    enum class BufferMemoryUsage
    {
        Unknown = 0,
        GPU, // GPU Only
        CPU, // CPU Only
        CPUToGPU, // CPU to GPU
        GPUToCPU, // GPU to CPU
        CPUCopy // CPU copy operations
    };

    // A general purpose buffer specification, usable for all buffer types
    // Note: This is only used in the constructor and not retrievable afterwards.
    struct BufferSpecification
    {
    public:
        BufferMemoryUsage Usage = BufferMemoryUsage::GPU;
    };

	///////////////////////////////////////////////////////////
	// Buffers
	///////////////////////////////////////////////////////////
	class VertexBuffer : public RefCounted
	{
	public:
        using VertexBufferType = VulkanVertexBuffer;
        static_assert(std::is_same_v<VertexBufferType, VulkanVertexBuffer>, "Unsupported vertexbuffer type selected.");
    public:
		VertexBuffer(const BufferSpecification& specs, void* data, size_t size);
		~VertexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const;
        static void Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers);

        // Returns underlying type pointer
        inline VertexBufferType* Src() { return m_Instance; }

		static Ref<VertexBuffer> Create(const BufferSpecification& specs, void* data, size_t size);

    private:
        VertexBufferType* m_Instance;
	};

	class IndexBuffer : public RefCounted
	{
	public:
        using IndexBufferType = VulkanIndexBuffer;
        static_assert(std::is_same_v<IndexBufferType, VulkanIndexBuffer>, "Unsupported indexbuffer type selected.");
    public:
		IndexBuffer(const BufferSpecification& specs, uint32_t* indices, uint32_t count);
		~IndexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const;

		uint32_t GetCount() const;

        // Returns underlying type pointer
        inline IndexBufferType* Src() { return m_Instance; }

		static Ref<IndexBuffer> Create(const BufferSpecification& specs, uint32_t* indices, uint32_t count);

    private:
        IndexBufferType* m_Instance;
	};

	// Note: Needs to be created after the pipeline
	class UniformBuffer : public RefCounted
	{
	public:
        using UniformBufferType = VulkanUniformBuffer;
        static_assert(std::is_same_v<UniformBufferType, VulkanUniformBuffer>, "Unsupported uniformbuffer type selected.");
    public:
		UniformBuffer(const BufferSpecification& specs, size_t dataSize);
		~UniformBuffer();

		void SetData(void* data, size_t size, size_t offset = 0);

        size_t GetSize() const;

        // Returns underlying type pointer
        inline UniformBufferType* Src() { return m_Instance; }

		static Ref<UniformBuffer> Create(const BufferSpecification& specs, size_t dataSize);

    private:
        UniformBufferType* m_Instance;
	};

    /* // TODO: To be implemented. (Come up with a better design)
	class DynamicUniformBuffer : public RefCounted
	{
	public:
		DynamicUniformBuffer();
		~DynamicUniformBuffer();

	    void SetData(void* data, size_t size);

		virtual void SetDataIndexed(uint32_t index, void* data, size_t size) = 0;
		virtual void UploadIndexedData() = 0;

		virtual uint32_t GetAmountOfElements() const = 0;
		virtual size_t GetAlignment() const = 0;

		//virtual void Upload(Ref<DescriptorSet> set, Descriptor element) = 0;
		//virtual void Upload(Ref<DescriptorSet> set, Descriptor element, size_t offset) = 0;

        // Returns underlying type pointer
        inline UniformBufferType* Src() { return m_Instance; }

		static Ref<DynamicUniformBuffer> Create(uint32_t elements, size_t sizeOfOneElement);
	};
    */

	class StorageBuffer : public RefCounted
	{
	public:
        using StorageBufferType = VulkanStorageBuffer;
        static_assert(std::is_same_v<StorageBufferType, VulkanStorageBuffer>, "Unsupported storagebuffer type selected.");
    public:
		StorageBuffer(const BufferSpecification& specs, size_t dataSize);
		~StorageBuffer();

		void SetData(void* data, size_t size, size_t offset = 0);

		size_t GetSize() const;

        // Returns underlying type pointer
        inline StorageBufferType* Src() { return m_Instance; }

		static Ref<StorageBuffer> Create(const BufferSpecification& specs, size_t dataSize);

    private:
        StorageBufferType* m_Instance;
	};

}
