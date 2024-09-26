#include "hzpch.h"
#include "Renderer2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/RenderSpace/2D/Resources2D.hpp"
#include "Horizon/RenderSpace/2D/BatchRenderer2D.hpp"

namespace Hz
{

	void Renderer2D::Init()
	{
		Resources2D::Init();
		BatchRenderer2D::Init();
	}

	void Renderer2D::Destroy()
	{
		BatchRenderer2D::Destroy();
		Resources2D::Destroy();
	}

	void Renderer2D::Resize(uint32_t width, uint32_t height)
	{
		BatchRenderer2D::Resize(width, height);
	}

	void Renderer2D::BeginBatch(const glm::mat4& projection, const glm::mat4& view)
	{
		auto& resources = Resources2D::Get();

		// Set buffer data
		std::array<const glm::mat4, 2> data = { view, projection };
		resources.Camera.Buffer->SetData((void*)data.data(), sizeof(glm::mat4) * 2, 0);

		BatchRenderer2D::Begin();
	}

	void Renderer2D::EndBatch()
	{
		BatchRenderer2D::End();
		BatchRenderer2D::Flush();
	}

}