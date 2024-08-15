#include "hzpch.h"
#include "hzpch.h"
#include "VulkanCommandBuffer.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanRenderer.hpp"

namespace Hz
{

	VulkanCommandBuffer::VulkanCommandBuffer()
	{
		auto device = VulkanContext::GetDevice()->GetVkDevice();
		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;
		m_CommandBuffers.resize(framesInFlight);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanContext::GetSwapChain()->GetVkCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data()));

		m_RenderFinishedSemaphores.resize(framesInFlight);
		m_InFlightFences.resize(framesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < framesInFlight; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
			VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]));
		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        Renderer::Free([commandBuffers = m_CommandBuffers, renderFinishedSemaphores = m_RenderFinishedSemaphores, inFlightFences = m_InFlightFences]()
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            vkFreeCommandBuffers(VulkanContext::GetDevice()->GetVkDevice(), VulkanContext::GetSwapChain()->GetVkCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

            for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
            {
                // Note: In obscure ways this sometimes gets called after the renderer is destroyed.
                // I don't want to impose some kind of lifetime rules, so this is the solution.
                if (Renderer::Initialized())
                {
                    VulkanRenderer::GetTaskManager().RemoveFromAll(renderFinishedSemaphores[i]);
                    VulkanRenderer::GetTaskManager().RemoveFromAll(inFlightFences[i]);
                }

                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(device, inFlightFences[i], nullptr);
            }
        });
	}



	VulkanCommand::VulkanCommand(bool start)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanContext::GetSwapChain()->GetVkCommandPool();
		allocInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice()->GetVkDevice(), &allocInfo, &m_CommandBuffer));

		if (start)
			Begin();
	}

	VulkanCommand::~VulkanCommand()
	{
		vkFreeCommandBuffers(VulkanContext::GetDevice()->GetVkDevice(), VulkanContext::GetSwapChain()->GetVkCommandPool(), 1, &m_CommandBuffer);
	}

	void VulkanCommand::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
	}

	void VulkanCommand::End()
	{
		vkEndCommandBuffer(m_CommandBuffer);
	}

	void VulkanCommand::Submit()
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

        auto queue = VulkanContext::GetDevice()->GetGraphicsQueue();
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
	}

	void VulkanCommand::EndAndSubmit()
	{
		End();
		Submit();
	}

}
