#include "hzpch.h"
#include "Resources2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Core/Application.hpp"

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

		s_Instance->InitMain();
		s_Instance->InitCamera();
		s_Instance->InitBatch();
	}

	void Resources2D::Destroy()
	{
		s_Instance->DestroyBatch();
		s_Instance->DestroyCamera();
		s_Instance->DestroyMain();

		s_Instance.Reset();
	}

	Resources2D& Resources2D::Get()
	{
		return *s_Instance.Raw();
	}

	//////////////////////////////////////////////////////////
	// Methods
	//////////////////////////////////////////////////////////
	void Resources2D::InitMain()
	{
		Main.WhiteTexture = Image::Create({ 1, 1, ImageUsageFlags::Colour | ImageUsageFlags::Sampled }, {});

		uint32_t white = 0xFFFFFFFF;
		Main.WhiteTexture->SetData(&white, sizeof(uint32_t));
	}

	void Resources2D::InitCamera()
	{
		Camera.Buffer = UniformBuffer::Create({ .Usage = BufferMemoryUsage::CPUToGPU }, (sizeof(glm::mat4) * 2)); // For View & Projection
	}

	void Resources2D::InitBatch()
	{
		std::vector<uint32_t> indices;
		indices.reserve(static_cast<size_t>(BatchRenderer2D::MaxQuads) * 6);

		for (uint32_t i = 0, offset = 0; i < BatchRenderer2D::MaxQuads * 6; i += 6, offset += 4)
		{
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);

			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 0);
		}

		Batch.RenderpassObject = Renderpass::Create({
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

		Batch.ShaderObject = Shader::Create({
			.ShaderCode = {
				{ ShaderStage::Vertex, ShaderCompiler::Compile(ShaderStage::Vertex, Shader::ReadGLSL(Application::Get().GetWorkingDir() / "Sandbox/Shaders/shader.vert.glsl")) },
				{ ShaderStage::Fragment, ShaderCompiler::Compile(ShaderStage::Fragment, Shader::ReadGLSL(Application::Get().GetWorkingDir() / "Sandbox/Shaders/shader.frag.glsl")) }
			}
		});

		Batch.DescriptorSetsObject = DescriptorSets::Create({
			{ 1, { 0, {
				{ DescriptorType::UniformBuffer, 0, "u_Camera", ShaderStage::Vertex },
				{ DescriptorType::CombinedImageSampler, 1, "u_Textures", ShaderStage::Fragment, BatchRenderer2D::MaxTextures, DescriptorBindingFlags::Default },
			}}}
		});

		Batch.PipelineObject = Pipeline::Create({
			.Type = PipelineType::Graphics,
			.Bufferlayout = BatchVertex2D::GetLayout(),
			.Polygonmode = PolygonMode::Fill,
			.Cullingmode = CullingMode::None,
			// .Blending = true
		}, Batch.DescriptorSetsObject, Batch.ShaderObject, Batch.RenderpassObject);

		Batch.VertexBufferObject = VertexBuffer::Create({ .Usage = BufferMemoryUsage::CPUToGPU }, nullptr, sizeof(BatchVertex2D) * BatchRenderer2D::MaxQuads * 4);
		Batch.IndexBufferObject = IndexBuffer::Create({}, indices.data(), static_cast<uint32_t>(indices.size()));

		// Reserve enough space for cpu buffer
		Batch.CPUBuffer.reserve(static_cast<size_t>(BatchRenderer2D::MaxQuads) * 4);
	}

	void Resources2D::DestroyMain()
	{
	}

	void Resources2D::DestroyCamera()
	{
	}

	void Resources2D::DestroyBatch()
	{
	}

}
