#pragma once

#include "Horizon/Core/Core.hpp"

namespace Hz
{

	class Renderer2D
	{
	public:
		static void Init();
		static void Destroy();

	public:
		static void BeginBatch();
		static void EndBatch();
	};

}