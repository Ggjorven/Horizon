#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Buffers.hpp"

#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Hz
{
	
	struct StaticMeshVertex
	{
	public:
		glm::vec3 Position = { };
		glm::vec2 TexCoord = { };
		glm::vec3 Normal = { };

	public:
		static BufferLayout GetLayout();
	};

	class StaticMesh : public RefCounted
	{
	public:
		StaticMesh() = default;
		StaticMesh(const std::filesystem::path& path);
		~StaticMesh() = default;

		inline Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

		static Ref<StaticMesh> Create(const std::filesystem::path& path);

	private:
		Ref<VertexBuffer> m_VertexBuffer = nullptr;
		Ref<IndexBuffer> m_IndexBuffer = nullptr;
	};

}
