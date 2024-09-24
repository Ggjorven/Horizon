#include "hzpch.h"
#include "BatchRenderer2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/RenderSpace/2D/Resources2D.hpp"

namespace Hz
{

	BufferLayout BatchVertex2D::GetLayout()
	{
		// Note: Has to be manually updated
		return {
			{ DataType::Float3, 0, "Position" },
			{ DataType::Float2, 1, "UV" },
			{ DataType::Float4, 2, "Colour" },
		};
	}



	void BatchRenderer2D::Init()
	{
	}

	void BatchRenderer2D::Destroy()
	{
	}

	void BatchRenderer2D::Begin()
	{
		auto& resources = Resources2D::Get().Batch;

		resources.CPUBuffer.clear();

		resources.DescriptorSetsObject->GetSets(0)[0]->Upload({
			{ Resources2D::Get().Camera.Buffer, resources.DescriptorSetsObject->GetLayout(0).GetDescriptorByName("u_Camera") }
		});
	}

	void BatchRenderer2D::End()
	{
		auto& resources = Resources2D::Get().Batch;

		// Only draw if there's something TO draw
		if (resources.CPUBuffer.size() > 0)
			resources.VertexBufferObject->SetData((void*)resources.CPUBuffer.data(), (resources.CPUBuffer.size() * sizeof(BatchVertex2D)), 0);
	}

	void BatchRenderer2D::Flush()
	{
		auto& resources = Resources2D::Get().Batch;

		auto& colourAttachment = GraphicsContext::GetSwapChainImages()[Renderer::GetAcquiredImage()];

		DynamicRenderState state = {
			.ColourAttachment = colourAttachment,
			.ColourLoadOp = LoadOperation::Clear,
			.ColourStoreOp = StoreOperation::Store,
			.ColourClearValue = { 0.0f, 0.0f, 0.0f, 1.0f },

			.DepthAttachment = GraphicsContext::GetDepthImage(),
			.DepthLoadOp = LoadOperation::Clear,
			.DepthStoreOp = StoreOperation::Store,
			.DepthClearValue = 1.0f
		};

		colourAttachment->Transition(ImageLayout::PresentSrcKHR, ImageLayout::Colour);

		// Start rendering
		Renderer::Begin(resources.CommandBufferObject);
		Renderer::BeginDynamic(resources.CommandBufferObject, std::move(state));
        Renderer::SetViewportAndScissor(resources.CommandBufferObject, colourAttachment->GetSpecification().Width, colourAttachment->GetSpecification().Height);

		resources.PipelineObject->Use(resources.CommandBufferObject, PipelineBindPoint::Graphics);

		resources.DescriptorSetsObject->GetSets(0)[0]->Bind(resources.PipelineObject, resources.CommandBufferObject);

		resources.IndexBufferObject->Bind(resources.CommandBufferObject);
		resources.VertexBufferObject->Bind(resources.CommandBufferObject);

		// Draw all at once
		Renderer::DrawIndexed(resources.CommandBufferObject, static_cast<uint32_t>(((resources.CPUBuffer.size() / 4ull) * 6ull)), 1);

		// End rendering
		Renderer::EndDynamic(resources.CommandBufferObject);
		Renderer::End(resources.CommandBufferObject);
		Renderer::Submit(resources.CommandBufferObject);

		colourAttachment->Transition(ImageLayout::Colour, ImageLayout::PresentSrcKHR);
	}

	void BatchRenderer2D::AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		constexpr const glm::vec2 uv0(1.0f, 0.0f);
		constexpr const glm::vec2 uv1(0.0f, 0.0f);
		constexpr const glm::vec2 uv2(0.0f, 1.0f);
		constexpr const glm::vec2 uv3(1.0f, 1.0f);

		auto& resources = Resources2D::Get().Batch;

		if ((resources.CPUBuffer.size() / 4u) >= BatchRenderer2D::MaxQuads)
		{
			End();
			Flush();
			Begin();
		}

		resources.CPUBuffer.emplace_back(position, uv0, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x + size.x, position.y, position.z), uv1, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x + size.x, position.y + size.y, position.z), uv2, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x, position.y + size.y, position.z), uv3, colour);
	}

}
