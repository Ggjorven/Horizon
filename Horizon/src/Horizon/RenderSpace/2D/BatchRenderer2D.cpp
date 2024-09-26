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

	void BatchRenderer2D::Resize(uint32_t width, uint32_t height)
	{
		auto& resources = Resources2D::Get().Batch;

		resources.RenderpassObject->Resize(width, height);
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

		// Start rendering
		Renderer::Begin(resources.RenderpassObject);

		auto cmdBuf = resources.RenderpassObject->GetCommandBuffer();

		resources.PipelineObject->Use(cmdBuf, PipelineBindPoint::Graphics);

		resources.DescriptorSetsObject->GetSets(0)[0]->Bind(resources.PipelineObject, cmdBuf);

		resources.IndexBufferObject->Bind(cmdBuf);
		resources.VertexBufferObject->Bind(cmdBuf);

		// Draw all at once
		Renderer::DrawIndexed(cmdBuf, static_cast<uint32_t>(((resources.CPUBuffer.size() / 4ull) * 6ull)), 1);

		// End rendering
		Renderer::End(resources.RenderpassObject);
		Renderer::Submit(resources.RenderpassObject);
	}

	void BatchRenderer2D::AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		constexpr const glm::vec2 uv0(1.0f, 0.0f);
		constexpr const glm::vec2 uv1(0.0f, 0.0f);
		constexpr const glm::vec2 uv2(0.0f, 1.0f);
		constexpr const glm::vec2 uv3(1.0f, 1.0f);

		auto& resources = Resources2D::Get().Batch;

		if ((resources.CPUBuffer.size() / 4u) >= BatchRenderer2D::MaxQuads) [[unlikely]]
		{
			HZ_LOG_WARN("Reached max amount of quads ({0}), to support more either manually change BatchRenderer2D::MaxQuads or contact the developer.", BatchRenderer2D::MaxQuads);
			return;
		}

		resources.CPUBuffer.emplace_back(position, uv0, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x + size.x, position.y, position.z), uv1, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x + size.x, position.y + size.y, position.z), uv2, colour);
		resources.CPUBuffer.emplace_back(glm::vec3(position.x, position.y + size.y, position.z), uv3, colour);
	}

}
