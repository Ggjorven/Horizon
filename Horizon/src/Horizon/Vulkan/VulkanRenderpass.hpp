#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Hz
{

    class VulkanRenderer;

	class VulkanRenderpass : public Renderpass
	{
	public:
		VulkanRenderpass(RenderpassSpecification specs, Ref<CommandBuffer> commandBuffer);
		~VulkanRenderpass();

        // The Begin, End & Submit methods are in the Renderer

		void Resize(uint32_t width, uint32_t height) override;

        std::pair<uint32_t, uint32_t> GetSize() const override;

		inline const RenderpassSpecification& GetSpecification() override { return m_Specification; }
		inline Ref<CommandBuffer> GetCommandBuffer() override { return m_CommandBuffer; }

		inline const VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
		inline std::vector<VkFramebuffer>& GetVkFrameBuffers() { return m_Framebuffers; };

	private:
		void CreateRenderpass();
		void CreateFramebuffers(uint32_t width, uint32_t height);
		void Destroy();

	private:
		RenderpassSpecification m_Specification;

		Ref<CommandBuffer> m_CommandBuffer;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_Framebuffers = { };

        friend class VulkanRenderer;
	};

}
