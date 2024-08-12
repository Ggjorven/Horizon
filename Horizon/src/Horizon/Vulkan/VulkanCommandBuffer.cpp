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
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		auto device = context.GetDevice()->GetVkDevice();
		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;
		m_CommandBuffers.resize(framesInFlight);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = context.GetSwapChain()->GetVkCommandPool();
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
            const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());
            VulkanRenderer* renderer = HzCast(VulkanRenderer, Renderer::Src());

            vkFreeCommandBuffers(context.GetDevice()->GetVkDevice(), context.GetSwapChain()->GetVkCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

            for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
            {
                if (renderer)
                {
                    renderer->GetTaskManager().RemoveFromAll(renderFinishedSemaphores[i]);
                    renderer->GetTaskManager().RemoveFromAll(inFlightFences[i]);
                }

                vkDestroySemaphore(context.GetDevice()->GetVkDevice(), renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(context.GetDevice()->GetVkDevice(), inFlightFences[i], nullptr);
            }
        });
	}



	VulkanCommand::VulkanCommand(bool start)
	{
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = context.GetSwapChain()->GetVkCommandPool();
		allocInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(context.GetDevice()->GetVkDevice(), &allocInfo, &m_CommandBuffer));

		if (start)
			Begin();
	}

	VulkanCommand::~VulkanCommand()
	{
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		vkFreeCommandBuffers(context.GetDevice()->GetVkDevice(), context.GetSwapChain()->GetVkCommandPool(), 1, &m_CommandBuffer);
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
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		vkQueueSubmit(context.GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(context.GetDevice()->GetGraphicsQueue());
	}

	void VulkanCommand::EndAndSubmit()
	{
		End();
		Submit();
	}

}
