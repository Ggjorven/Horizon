#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Buffers.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Hz
{

	struct BatchVertex2D
	{
	public:
		constexpr static uint32_t MaxQuadsPerDraw = 10000u;
	public:
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f }; // TODO: Check if Z should be 0.0f or 1.0f
		glm::vec2 UV = { 0.0f, 0.0f };
		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

	public:
		static BufferLayout GetLayout();
	};

	class BatchRenderer2D
	{
	public:
		static void Init();
		static void Destroy();

	public:
		static void Begin();
		static void End();
		static void Flush();

		static void AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour);
	};

}