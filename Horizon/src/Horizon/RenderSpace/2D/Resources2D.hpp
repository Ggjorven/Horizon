#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Pipeline.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/RenderSpace/2D/BatchRenderer2D.hpp"

#include <vector>

namespace Hz
{

	class Resources2D
	{
	///////////////////////////////////////////////////////////
	// Main functions
	///////////////////////////////////////////////////////////
	public:
		static void Init();
		static void Destroy();

		static Resources2D& Get();

	///////////////////////////////////////////////////////////
	// Data
	///////////////////////////////////////////////////////////
	public:
		struct
		{
			Ref<UniformBuffer> Buffer = nullptr;
		} Camera;

		struct // TODO: Use static rendering.
		{
			Ref<Pipeline> PipelineObject = nullptr;
			Ref<Shader> ShaderObject = nullptr;
			Ref<DescriptorSets> DescriptorSetsObject = nullptr;

			Ref<CommandBuffer> CommandBufferObject = nullptr;

			Ref<VertexBuffer> VertexBufferObject = nullptr;
			Ref<IndexBuffer> IndexBufferObject = nullptr;

			std::vector<BatchVertex2D> CPUBuffer = {};
		} Batch;

	///////////////////////////////////////////////////////////
	// Methods
	///////////////////////////////////////////////////////////
	private:
		void InitCamera();
		void InitBatch();

		void DestroyCamera();
		void DestroyBatch();
	};

}
