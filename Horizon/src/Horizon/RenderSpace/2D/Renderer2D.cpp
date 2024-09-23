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

	void Renderer2D::BeginBatch()
	{
		BatchRenderer2D::Begin();
	}

	void Renderer2D::EndBatch()
	{
		BatchRenderer2D::End();
		BatchRenderer2D::Flush();
	}

}