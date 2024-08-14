#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Hz
{

    class VulkanRenderer;

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer();
		virtual ~VulkanCommandBuffer();

		// The Begin, End & Submit methods are in the Renderer class.

		inline const VkSemaphore GetVkRenderFinishedSemaphore(uint32_t index) const { return m_RenderFinishedSemaphores[index]; }
		inline const VkFence GetVkInFlightFence(uint32_t index) const { return m_InFlightFences[index]; }
		inline const VkCommandBuffer GetVkCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }

	private:
		std::vector<VkCommandBuffer> m_CommandBuffers = { };

		// Synchronization objects
		std::vector<VkSemaphore> m_RenderFinishedSemaphores = { };
		std::vector<VkFence> m_InFlightFences = { };

        friend class VulkanRenderer;
	};



	class VulkanCommand // For running simple commands
	{
	public:
		VulkanCommand(bool start = false);
		virtual ~VulkanCommand();

		void Begin();
		void End();
		void Submit();
		void EndAndSubmit();

		inline const VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};

}
