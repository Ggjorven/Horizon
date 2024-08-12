#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Image.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <initializer_list>
#include <type_traits>

namespace Hz
{

    class VulkanDescriptorSet;
    class VulkanDescriptorSets;

    using namespace Pulse::Enum::Bitwise;

	class Pipeline;
	class CommandBuffer;

	///////////////////////////////////////////////////////////
	// Specifications
	///////////////////////////////////////////////////////////
    enum class DescriptorType : uint32_t
    {
        None = 0x7FFFFFFF,
        Sampler = 0,
        CombinedImageSampler = 1,
        SampledImage = 2,
        StorageImage = 3,
        UniformTexelBuffer = 4,
        StorageTexelBuffer = 5,
        UniformBuffer = 6,
        StorageBuffer = 7,
        DynamicUniformBuffer = 8,
        DynamicStorageBuffer = 9,
        InputAttachment = 10,
        UniformBlock = 1000138000,
        AccelerationStructureKHR = 1000150000,
        AccelerationStructureNV = 1000165000,
        SampleWeightImageQCOM = 1000440000,
        BlockMatchImageQCOM = 1000440001,
        MutableEXT = 1000351000,
        UniformBlockEXT = UniformBlock,
        MutableValve = MutableEXT,
    };

	enum class ShaderStage : uint32_t
	{
        None = 0,
		Vertex = 0x00000001,
        TessellationControl = 0x00000002,
        TessellationEvaluation = 0x00000004,
        Geometry = 0x00000008,
        Fragment = 0x00000010,
        Compute = 0x00000020,
        AllGraphics = 0x0000001F,
        All = 0x7FFFFFFF,
        RayGenKHR = 0x00000100,
        AnyHitKHR = 0x00000200,
        ClosestHitKHR = 0x00000400,
        MissKHR = 0x00000800,
        IntersectionKHR = 0x00001000,
        CallableKHR = 0x00002000,
        TaskEXT = 0x00000040,
        MeshEXT = 0x00000080,
        SubpassShadingHuawei = 0x00004000,
        ClusterCullingHuawei = 0x00080000,
        RayGenNV = RayGenKHR,
        AnyHitNV = AnyHitKHR,
        ClosestHitNV = ClosestHitKHR,
        MissNV = MissKHR,
        IntersectionNV = IntersectionKHR,
        CallableNV = CallableKHR,
        TaskNV = TaskEXT,
        MeshNV = MeshEXT
	};

    enum class PipelineBindPoint
    {
        Graphics = 0,
        Compute = 1,
        RayTracingKHR = 1000165000,
        SubpassShadingHuawei = 1000369003,
        RayTracingNV = RayTracingKHR
    };

	// Note: You can think of a descriptor as a uniform or some variable in the shader
	struct Descriptor
	{
	public:
		std::string Name = "Empty";
		uint32_t Binding = 0;
		DescriptorType Type = DescriptorType::None;
		uint32_t Count = 1;
		ShaderStage Stage = ShaderStage::None;

		Descriptor() = default;
		Descriptor(DescriptorType type, uint32_t binding, const std::string& name, ShaderStage stage, uint32_t count = 1);
		~Descriptor() = default;
	};

    struct DescriptorSetLayout
	{
	public:
		uint32_t SetID = 0;
		std::unordered_map<std::string, Descriptor> Descriptors = { };

	public:
		DescriptorSetLayout() = default;
		DescriptorSetLayout(uint32_t setID, const std::vector<Descriptor>& descriptors);
		DescriptorSetLayout(uint32_t setID, const std::initializer_list<Descriptor>& descriptors);
		~DescriptorSetLayout() = default;

		Descriptor GetDescriptorByName(const std::string& name) const;
		std::unordered_set<DescriptorType> UniqueTypes() const;
		uint32_t AmountOf(DescriptorType type) const;
	};

    // A simple specification class for specifying the layout + amount of descriptor sets of a certain ID
    struct DescriptorSetGroup
    {
    public:
        uint32_t Amount = 1;
        DescriptorSetLayout Layout = {};

    public:
        DescriptorSetGroup() = default;
        DescriptorSetGroup(uint32_t amount, const DescriptorSetLayout& layout);
        ~DescriptorSetGroup() = default;
    };

    struct Uploadable
    {
    public:
        using Type = std::variant<Ref<Image>, Ref<UniformBuffer>, Ref<StorageBuffer>>;
    public:
        Type Value;
        Descriptor Element;

    public:
        Uploadable(Type value, Descriptor element);
        ~Uploadable() = default;
    };

    ///////////////////////////////////////////////////////////
	// Core class
	///////////////////////////////////////////////////////////
    class DescriptorSet : public RefCounted
	{
	public:
        using DescriptorSetType = VulkanDescriptorSet;
        static_assert(std::is_same_v<DescriptorSetType, VulkanDescriptorSet>, "Unsupported descriptorset type selected.");
    public:
		DescriptorSet(DescriptorSetType* src);
		~DescriptorSet();

		void Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics, const std::vector<uint32_t>& dynamicOffsets = { });

        void Upload(const std::initializer_list<Uploadable>& elements);

        // Returns underlying type pointer
        inline DescriptorSetType* Src() { return m_Instance; }

    private:
        DescriptorSetType* m_Instance;
    };

	class DescriptorSets : public RefCounted
	{
	public:
        using DescriptorSetsType = VulkanDescriptorSets;
        static_assert(std::is_same_v<DescriptorSetsType, VulkanDescriptorSets>, "Unsupported descriptorsets type selected.");
    public:
		DescriptorSets(const std::initializer_list<DescriptorSetGroup>& specs);
		DescriptorSets(DescriptorSetsType* src);
		~DescriptorSets();

        void SetAmountOf(uint32_t setID, uint32_t amount); // Create amount of setID's
		uint32_t GetAmountOf(uint32_t setID) const;

		const DescriptorSetLayout& GetLayout(uint32_t setID) const;
		std::vector<Ref<DescriptorSet>>& GetSets(uint32_t setID);

        // Returns underlying type pointer
        inline DescriptorSetsType* Src() { return m_Instance; }

		static Ref<DescriptorSets> Create(const std::initializer_list<DescriptorSetGroup>& specs);

    private:
        DescriptorSetsType* m_Instance;
	};

}
