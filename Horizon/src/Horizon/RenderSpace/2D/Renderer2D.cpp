#include "hzpch.h"
#include "Renderer2D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/RenderSpace/2D/Resources2D.hpp"

namespace Hz
{

	void Renderer2D::Init()
	{
		HZ_ASSERT(false, "2D Rendering is yet to be implemented.");
		Resources2D::Init();
	}

	void Renderer2D::Destroy()
	{
		HZ_ASSERT(false, "2D Rendering is yet to be implemented.");
		Resources2D::Destroy();
	}

}