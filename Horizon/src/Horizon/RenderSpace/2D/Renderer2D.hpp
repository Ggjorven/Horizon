#pragma once

#include "Horizon/Core/Core.hpp"

#include <glm/glm.hpp>

namespace Hz
{

	class Renderer2D
	{
	public:
		static void Init();
		static void Destroy();

		static void Resize(uint32_t width, uint32_t height);

	public:
		static void BeginBatch(const glm::mat4& projection = glm::mat4(1.0f), const glm::mat4& view = glm::mat4(1.0f));
		static void EndBatch();
	};

}