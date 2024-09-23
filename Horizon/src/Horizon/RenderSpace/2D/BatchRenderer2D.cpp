#include "hzpch.h"
#include "BatchRenderer2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"

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

		Renderer::Begin(resources.Renderpass);

		// Start rendering
		auto cmdBuf = resources.Renderpass->GetCommandBuffer();

		resources.Pipeline->Use(cmdBuf, PipelineBindPoint::Graphics);

		resources.IndexBuffer->Bind(cmdBuf);
		resources.VertexBuffer->Bind(cmdBuf);

		// Draw all at once
		Renderer::DrawIndexed(cmdBuf, ((resources.CPUBuffer.size() / 4u) * 6u), 1);

		// End rendering
		Renderer::End(resources.Renderpass);
		Renderer::Submit(resources.Renderpass);
	}

	void BatchRenderer2D::AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		auto& resources = Resources2D::Get().Batch;

		if ((resources.CPUBuffer.size() / 4u) >= BatchVertex2D::MaxQuadsPerDraw)
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