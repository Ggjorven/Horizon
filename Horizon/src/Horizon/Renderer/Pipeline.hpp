#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

namespace Hz
{

    class VulkanPipeline;

	class Renderpass;
	class DescriptorSets;
	class CommandBuffer;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Specification
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	enum class CullingMode : uint8_t
	{
		None = 0,
        Front,
        Back,
        FrontAndBack
	};

	enum class PolygonMode
    {
        None = 0x7FFFFFFF,
        Fill = 0,
        Line = 1,
        Point = 2,
        FillRectangleNV = 1000153000,
    };

    enum class PipelineType
    {
        Graphics = 0,
        Compute,
        RayTracingKHR,
        RayTracingNV
    };

	struct PipelineSpecification
	{
	public:
        PipelineType Type = PipelineType::Graphics;

        // Graphics
		BufferLayout Bufferlayout = {};

		PolygonMode Polygonmode = PolygonMode::Fill;
		CullingMode Cullingmode = CullingMode::Front;

		float LineWidth = 1.0f;
		bool Blending = false;

        // Raytracing KHR
        uint32_t MaxRayRecursion = 1;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pipeline
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Pipeline : public RefCounted // Note: Pipeline does not keep the shader/descriptorsets/renderpass alive/in scope
	{
	public:
        using PipelineType = VulkanPipeline;
        static_assert(std::is_same_v<PipelineType, VulkanPipeline>, "Unsupported pipeline type selected.");
    public:
		Pipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass);
		Pipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader);
		~Pipeline();

		void Use(Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics);

        // Make sure a compute shader is present in the current pipeline and the pipeline is bound.
        void DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth);

		const PipelineSpecification& GetSpecification() const;

		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass); // Graphics constructor
		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader); // Compute/RayTracingKHR/RayTracingNV constructor

        // Returns underlying type pointer
        inline PipelineType* Src() { return m_Instance; }

    private:
        PipelineType* m_Instance;
	};

}
