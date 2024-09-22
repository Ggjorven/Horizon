#include "hzpch.h"
#include "StaticMesh.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Hz
{

	static void LoadModel(const std::filesystem::path& path, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices);
	static void ProcessNode(aiNode* node, const aiScene* scene, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices);
	static void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices);

	BufferLayout StaticMeshVertex::GetLayout()
	{
		return {
			BufferElement(DataType::Float3, 0, "a_Position"),
			BufferElement(DataType::Float2, 1, "a_TexCoord"),
			BufferElement(DataType::Float3, 2, "a_Normal")
		};
	}

	StaticMesh::StaticMesh(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			HZ_LOG_WARN("Mesh by path: '{0}' doesn't exist.", path.string());
			return;
		}

		std::vector<StaticMeshVertex> vertices = { };
		std::vector<uint32_t> indices = { };

		LoadModel(path, vertices, indices);

		m_VertexBuffer = VertexBuffer::Create({}, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
		m_IndexBuffer = IndexBuffer::Create({}, indices.data(), (uint32_t)indices.size());
	}

	Ref<StaticMesh> StaticMesh::Create(const std::filesystem::path& path)
	{
		return Ref<StaticMesh>::Create(path);
	}

	static void LoadModel(const std::filesystem::path& path, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices)
	{
		Assimp::Importer importer = {};

		uint32_t flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
		if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
			flags |= aiProcess_FlipUVs;

		const aiScene* scene = importer.ReadFile(path.string(), flags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			HZ_LOG_ERROR("Failed to load mesh from: \"{}\"", path.string());
			return;
		}

		ProcessNode(scene->mRootNode, scene, vertices, indices);
	}

	static void ProcessNode(aiNode* node, const aiScene* scene, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices)
	{
		// Process all the node's meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, vertices, indices);
		}

		// Then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene, vertices, indices);
	}

	static void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<StaticMeshVertex>& vertices, std::vector<uint32_t>& indices)
	{
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			StaticMeshVertex vertex = {};

			// Position
			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			// Texture coordinates
			if (mesh->HasTextureCoords(0))
			{
				glm::vec2 vec(0.0f);
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoord = vec;
			}
			else
				vertex.TexCoord = glm::vec2(0.0f, 0.0f);

			// Normals
			if (mesh->HasNormals())
			{
				vertex.Normal.x = mesh->mNormals[i].x;
				vertex.Normal.y = mesh->mNormals[i].y;
				vertex.Normal.z = mesh->mNormals[i].z;
			}
			else
				vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		// Index processing
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
	}


}
