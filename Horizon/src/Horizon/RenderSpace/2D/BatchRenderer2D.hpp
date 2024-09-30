#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Image.hpp"
#include "Horizon/Renderer/Buffers.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Hz
{

	struct BatchVertex2D
	{
	public:
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec2 UV = { 0.0f, 0.0f };
		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		uint32_t TextureID = 0; // 0 is the white texure

	public:
		static BufferLayout GetLayout();
	};

	class BatchRenderer2D
	{
	public:
		// Note: Subject to change at any time
		constexpr static uint32_t MaxQuads = 10000u;
		constexpr static uint32_t MaxTextures = 1024u;
	public:
		static void Init();
		static void Destroy();

		static void Resize(uint32_t width, uint32_t height);

	public:
		static void Begin();
		static void End();
		static void Flush();

		// Note: We multiply the Z-axis by -1, so the depth is from 0 to 1
		static void AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour);
		static void AddQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour, Ref<Image> texture);

	private:
		static uint32_t GetTextureID(Ref<Image> image);
	};

}