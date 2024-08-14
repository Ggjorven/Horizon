#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

namespace Hz
{

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
		Pipeline() = default;
		virtual ~Pipeline() = default;

		virtual void Use(Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics) = 0;

        // Make sure a compute shader is present in the current pipeline and the pipeline is bound.
        virtual void DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth) = 0;

		virtual const PipelineSpecification& GetSpecification() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass); // Only for graphics
		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader); // All pipeline types
	};

}
