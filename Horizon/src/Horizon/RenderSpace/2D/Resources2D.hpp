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

		struct
		{
			Ref<Pipeline> Pipeline = nullptr;
			Ref<Shader> Shader = nullptr;
			Ref<DescriptorSets> DescriptorSets = nullptr;

			Ref<CommandBuffer> CommandBuffer = nullptr;

			Ref<VertexBuffer> VertexBuffer = nullptr;
			Ref<IndexBuffer> IndexBuffer = nullptr;

			std::vector<BatchVertex2D> CPUBuffer = {};
			uint32_t DrawIndex = 0; // Used for having multiple batches and handling dynamic state
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