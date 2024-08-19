#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include <unordered_map>

namespace Hz
{

	class Renderpass;
	class DescriptorSets;
	class CommandBuffer;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Specification
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Note: Used in Renderer::Submit as a waitStage
    enum class PipelineStage
    {
        None = 0,
        TopOfPipe = 0x00000001,
        DrawIndirect = 0x00000002,
        VertexInput = 0x00000004,
        VertexShader = 0x00000008,
        TessellationControlShader = 0x00000010,
        TessellationEvaluationShader = 0x00000020,
        GeometryShader = 0x00000040,
        FragmentShader = 0x00000080,
        EarlyFragmentTests = 0x00000100,
        LateFragmentTests = 0x00000200,
        ColourAttachmentOutput = 0x00000400,
        ComputeShader = 0x00000800,
        Transfer = 0x00001000,
        BottomOfPipe = 0x00002000,
        Host = 0x00004000,
        AllGraphics = 0x00008000,
        AllCommands = 0x00010000,
        TransformFeedbackEXT = 0x01000000,
        ConditionalRenderingEXT = 0x00040000,
        AccelerationStructureBuildKHR = 0x02000000,
        RayTracingShaderKHR = 0x00200000,
        FragmentDensityProcessEXT = 0x00800000,
        FragmentShadingRateAttachmentKHR = 0x00400000,
        CommandPreprocessNV = 0x00020000,
        TaskShaderEXT = 0x00080000,
        MeshShaderEXT = 0x00100000,
        ShadingRateImageNV = FragmentShadingRateAttachmentKHR,
        RayTracingShaderNV = RayTracingShaderKHR,
        AccelerationStructureBuildNV = AccelerationStructureBuildKHR,
        TaskShaderNV = TaskShaderEXT,
        MeshShaderNV = MeshShaderEXT,
        NoneKHR = None,
    };

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

    // Global specification for push constants
    struct PushConstantsSpecification
    {
    public:
        size_t Offset = 0;
        size_t Size = 0;
    };

	struct PipelineSpecification
	{
	public:
        PipelineType Type = PipelineType::Graphics;

        // Note: Keep in mind that pushConstants most of the time only have a size of 128 bytes (two mat4's).
        // Note 2: Here's an example of how to use pushConstants across multiple shader stages:
        // pipelineSpecification.PushConstants[ShaderStage::Vertex | ShaderStage::Fragment] = {};
        std::unordered_map<ShaderStage, PushConstantsSpecification> PushConstants = { };

        // Graphics
		BufferLayout Bufferlayout = {};

		PolygonMode Polygonmode = PolygonMode::Fill;
		CullingMode Cullingmode = CullingMode::Front; // Note: Before blaming the culling mode, make sure you have depth in your renderpass ;)

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

        // Push entire constant of stage specified in PipelineSpecification.PushConstants
        virtual void PushConstant(Ref<CommandBuffer> commandBuffer, ShaderStage stage, void* data) = 0;
        // Push part of a constant manually
        virtual void PushConstant(Ref<CommandBuffer> commandBuffer, ShaderStage stage, void* data, size_t offset, size_t size) = 0;

        // Make sure a compute shader is present in the current pipeline and the pipeline is bound.
        virtual void DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth) = 0;

		virtual const PipelineSpecification& GetSpecification() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass); // Only for graphics
		static Ref<Pipeline> Create(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader); // All pipeline types
	};

}
