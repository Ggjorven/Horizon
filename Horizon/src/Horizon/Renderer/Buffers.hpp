#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <cstdint>
#include <string>

namespace Hz
{

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
		VertexBuffer() = default;
		virtual ~VertexBuffer() = default;

		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;
        static void Bind(Ref<CommandBuffer> commandBuffer, std::vector<Ref<VertexBuffer>>&& buffers);

		static Ref<VertexBuffer> Create(const BufferSpecification& specs, void* data, size_t size);
	};

	class IndexBuffer : public RefCounted
	{
    public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;

		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(const BufferSpecification& specs, uint32_t* indices, uint32_t count);
	};

	// Note: Needs to be created after the pipeline
	class UniformBuffer : public RefCounted
	{
    public:
		UniformBuffer() = default;
		virtual ~UniformBuffer() = default;

		virtual void SetData(void* data, size_t size, size_t offset = 0) = 0;

        virtual size_t GetSize() const = 0;

		static Ref<UniformBuffer> Create(const BufferSpecification& specs, size_t dataSize);
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
		StorageBuffer() = default;
		virtual ~StorageBuffer() = default;

		virtual void SetData(void* data, size_t size, size_t offset = 0) = 0;

		virtual size_t GetSize() const = 0;

		static Ref<StorageBuffer> Create(const BufferSpecification& specs, size_t dataSize);
	};

}
