#include "hzpch.h"
#include "Resources2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

namespace Hz
{

	static Unique<Resources2D> s_Instance = nullptr;



	//////////////////////////////////////////////////////////
	// Main functions
	//////////////////////////////////////////////////////////
	void Resources2D::Init()
	{
		s_Instance = Unique<Resources2D>::Create();
		s_Instance->InitBatch();
	}

	void Resources2D::Destroy()
	{
		s_Instance->DestroyBatch();
		s_Instance.Reset();
	}

	Resources2D& Resources2D::Get()
	{
		return *s_Instance.Raw();
	}

	//////////////////////////////////////////////////////////
	// Methods
	//////////////////////////////////////////////////////////
	void Resources2D::InitBatch()
	{
		// std::vector<BatchVertex2D> vertices;
		// vertices.reserve(static_cast<size_t>(BatchVertex2D::MaxQuadsPerDraw) * 4);
		std::vector<uint32_t> indices;
		indices.reserve(static_cast<size_t>(BatchVertex2D::MaxQuadsPerDraw) * 6);

		for (uint32_t i = 0, offset = 0; i < BatchVertex2D::MaxQuadsPerDraw * 6; i += 6, offset += 4)
		{
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 0);
		}

		Batch.Renderpass = Renderpass::Create({
			.ColourAttachment = GraphicsContext::GetSwapChainImages(),
			.ColourLoadOp = LoadOperation::Clear,
			.ColourStoreOp = StoreOperation::Store,
			.ColourClearColour { 0.0f, 0.0f, 0.0f, 1.0f },
			.PreviousColourImageLayout = ImageLayout::Undefined,
			.FinalColourImageLayout = ImageLayout::PresentSrcKHR,

			.DepthAttachment = GraphicsContext::GetDepthImage(),
			.DepthLoadOp = LoadOperation::Clear,
			.DepthStoreOp = StoreOperation::Store,
			.PreviousDepthImageLayout = ImageLayout::Undefined,
			.FinalDepthImageLayout = ImageLayout::DepthStencil,
		}, CommandBuffer::Create());

		Batch.Shader = Shader::Create({
			.ShaderCode = {
				{ ShaderStage::Vertex, ShaderCompiler::Compile(ShaderStage::Vertex, Shader::ReadGLSL("Sandbox/Shaders/shader.vert.glsl")) },
				{ ShaderStage::Fragment, ShaderCompiler::Compile(ShaderStage::Fragment, Shader::ReadGLSL("Sandbox/Shaders/shader.frag.glsl")) }
			}
		});
		
		Batch.DescriptorSets = DescriptorSets::Create({ 
			// TODO: Add sets
		});

		Batch.Pipeline = Pipeline::Create({
			.Type = PipelineType::Graphics,
			.Bufferlayout = BatchVertex2D::GetLayout(),
			.Polygonmode = PolygonMode::Fill,
			.Cullingmode = CullingMode::None,
			// .Blending = true
		}, Batch.DescriptorSets, Batch.Shader, Batch.Renderpass);

		Batch.VertexBuffer = VertexBuffer::Create({ .Usage = BufferMemoryUsage::CPUToGPU }, nullptr, sizeof(BatchVertex2D) * BatchVertex2D::MaxQuadsPerDraw * 4);
		Batch.IndexBuffer = IndexBuffer::Create({}, indices.data(), static_cast<uint32_t>(indices.size()));
	}

	void Resources2D::DestroyBatch()
	{
	}

}