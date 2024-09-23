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
		resources.DrawIndex++;

		resources.DescriptorSets->GetSets(0)[0]->Upload({ 
			{ Resources2D::Get().Camera.Buffer, resources.DescriptorSets->GetLayout(0).GetDescriptorByName("u_Camera") }
		});
	}

	void BatchRenderer2D::End()
	{
		auto& resources = Resources2D::Get().Batch;

		// Only draw if there's something TO draw
		if (resources.CPUBuffer.size() > 0)
			resources.VertexBuffer->SetData((void*)resources.CPUBuffer.data(), (resources.CPUBuffer.size() * sizeof(BatchVertex2D)), 0);
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

		// Set LoadOperation to Load on multiple draw batches
		if (resources.DrawIndex > 1) [[unlikely]]
		{
			state.ColourLoadOp = LoadOperation::Load;
			state.DepthLoadOp = LoadOperation::Load;
		}

		colourAttachment->Transition(ImageLayout::PresentSrcKHR, ImageLayout::Colour);

		// Start rendering
		Renderer::Begin(resources.CommandBuffer);
		Renderer::BeginDynamic(resources.CommandBuffer, std::move(state));
        Renderer::SetViewportAndScissor(resources.CommandBuffer, colourAttachment->GetSpecification().Width, colourAttachment->GetSpecification().Height);

		resources.Pipeline->Use(resources.CommandBuffer, PipelineBindPoint::Graphics);

		resources.DescriptorSets->GetSets(0)[0]->Bind(resources.Pipeline, resources.CommandBuffer);

		resources.IndexBuffer->Bind(resources.CommandBuffer);
		resources.VertexBuffer->Bind(resources.CommandBuffer);

		// Draw all at once
		Renderer::DrawIndexed(resources.CommandBuffer, static_cast<uint32_t>(((resources.CPUBuffer.size() / 4ull) * 6ull)), 1);

		// End rendering
		Renderer::EndDynamic(resources.CommandBuffer);
		Renderer::End(resources.CommandBuffer);
		Renderer::Submit(resources.CommandBuffer);

		colourAttachment->Transition(ImageLayout::Colour, ImageLayout::PresentSrcKHR);
	}

	void BatchRenderer2D::AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		auto& resources = Resources2D::Get().Batch;

		if ((resources.CPUBuffer.size() / 4u) >= BatchRenderer2D::MaxQuadsPerDraw)
		{
			End();
			Flush();
			Begin();
		}

		auto& p0 = position;
		auto p1 = glm::vec3(p0.x + size.x, p0.y, p0.z);
		auto p2 = glm::vec3(p0.x + size.x, p0.y + size.y, p0.z);
		auto p3 = glm::vec3(p0.x, p0.y + size.y, p0.z);

		resources.CPUBuffer.emplace_back(p0, glm::vec2(1.0f, 0.0f), colour);
		resources.CPUBuffer.emplace_back(p1, glm::vec2(0.0f, 0.0f), colour);
		resources.CPUBuffer.emplace_back(p2, glm::vec2(0.0f, 1.0f), colour);
		resources.CPUBuffer.emplace_back(p3, glm::vec2(1.0f, 1.0f), colour);
	}

}