#include "hzpch.h"
#include "Renderer3D.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/RenderSpace/3D/Resources3D.hpp"

namespace Hz
{

	void Renderer3D::Init()
	{
		HZ_ASSERT(false, "3D Rendering is not implemented.");
		Resources3D::Init();
	}

	void Renderer3D::Destroy()
	{
		HZ_ASSERT(false, "3D Rendering is not implemented.");
		Resources3D::Destroy();
	}

}