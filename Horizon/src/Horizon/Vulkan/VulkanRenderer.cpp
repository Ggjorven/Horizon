#include "hzpch.h"
#include "VulkanRenderer.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Image.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"
#include "Horizon/Vulkan/VulkanRenderpass.hpp"
#include "Horizon/Vulkan/VulkanBuffers.hpp"
#include "Horizon/Vulkan/VulkanImage.hpp"

#include <numeric>

namespace Hz
{

    VulkanRenderer::VulkanRenderer(const RendererSpecification& specs)
        : m_Specification(specs)
    {
    }

    VulkanRenderer::~VulkanRenderer()
    {
    }

    void VulkanRenderer::Recreate(uint32_t width, uint32_t height, const bool vsync)
    {
        VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        context.GetSwapChain()->Init(width, height, vsync, (uint8_t)Renderer::GetSpecification().Buffers);
        m_Specification.VSync = vsync;
    }

    void VulkanRenderer::BeginFrame()
    {
        if (Window::Get().IsMinimized())
            return;

        Renderer::FreeObjects();

        VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());
        auto swapChain = context.GetSwapChain();

        {
            auto& fences = m_Manager.GetFences();
            if (!fences.empty())
            {
                vkWaitForFences(context.GetDevice()->GetVkDevice(), (uint32_t)fences.size(), fences.data(), VK_TRUE, ULONG_LONG_MAX);
                vkResetFences(context.GetDevice()->GetVkDevice(), (uint32_t)fences.size(), fences.data());
            }
            m_Manager.ResetFences();

            m_Manager.Add(context.GetSwapChain()->GetCurrentImageAvailableSemaphore());
        }
        {
            // Acquire SwapChain Image
            uint32_t acquiredImage = swapChain->AcquireNextImage();
            swapChain->m_AcquiredImage = acquiredImage;
        }
    }

    void VulkanRenderer::EndFrame()
    {
        if (Window::Get().IsMinimized())
            return;
    }

    void VulkanRenderer::Present()
    {
        if (Window::Get().IsMinimized())
            return;

        VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        auto& semaphores = m_Manager.GetSemaphores();
        auto swapChain = context.GetSwapChain();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
		presentInfo.pWaitSemaphores = semaphores.data();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain->m_SwapChain;
		presentInfo.pImageIndices = &swapChain->m_AcquiredImage;
		presentInfo.pResults = nullptr; // Optional

		VkResult result = VK_SUCCESS;
		{
			// Note(Jorben): Without this line there is a memory leak on windows when validation layers are enabled.
            #if defined(HZ_PLATFORM_WINDOWS)
			if constexpr (VulkanContext::s_Validation)
			{
				vkQueueWaitIdle(context.GetDevice()->GetGraphicsQueue());
			}
            #endif

			result = vkQueuePresentKHR(context.GetDevice()->GetPresentQueue(), &presentInfo);
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			auto& window = Window::Get();
			Recreate(window.GetWidth(), window.GetHeight(), m_Specification.VSync);
		}
		else if (result != VK_SUCCESS)
		{
			HZ_LOG_ERROR("Failed to present swap chain image!");
		}

        m_Manager.ResetSemaphores();
		swapChain->m_CurrentFrame = (swapChain->m_CurrentFrame + 1) % (uint32_t)m_Specification.Buffers;
    }

    void VulkanRenderer::BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state)
    {
        VulkanCommandBuffer* vkCmdBuf = HzCast(VulkanCommandBuffer, cmdBuf->Src());

        VkRenderingAttachmentInfo colourAttachment = {};
        colourAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colourAttachment.imageView = (state.ColourAttachment ? HzCast(VulkanImage, state.ColourAttachment->Src())->GetVkImageView() : VK_NULL_HANDLE);
        colourAttachment.imageLayout = (state.ColourAttachment ? (VkImageLayout)state.ColourAttachment->GetSpecification().Layout : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colourAttachment.loadOp = (VkAttachmentLoadOp)state.ColourLoadOp;
        colourAttachment.storeOp = (VkAttachmentStoreOp)state.ColourStoreOp;
        colourAttachment.clearValue = { state.ColourClearValue.r, state.ColourClearValue.g, state.ColourClearValue.b, state.ColourClearValue.a };

        VkRenderingAttachmentInfo depthAttachment = {};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = (state.DepthAttachment ? HzCast(VulkanImage, state.DepthAttachment->Src())->GetVkImageView() : VK_NULL_HANDLE);
        depthAttachment.imageLayout = (state.DepthAttachment ? (VkImageLayout)state.DepthAttachment->GetSpecification().Layout : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        depthAttachment.loadOp = (VkAttachmentLoadOp)state.DepthLoadOp;
        depthAttachment.storeOp = (VkAttachmentStoreOp)state.DepthStoreOp;
        depthAttachment.clearValue = { state.DepthClearValue };

        uint32_t width = 0, height = 0;
        if (state.ColourAttachment)
        {
            width = state.ColourAttachment->GetSpecification().Width;
            height = state.ColourAttachment->GetSpecification().Height;
        }
        else if (state.DepthAttachment)
        {
            width = state.DepthAttachment->GetSpecification().Width;
            height = state.DepthAttachment->GetSpecification().Height;
        }
        else
        {
            HZ_ASSERT(false, "No Colour or Depth attachment passed in to BeginDynamic(..., state)");
        }

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = { width, height };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = (state.ColourAttachment ? 1 : 0);
        renderingInfo.pColorAttachments = (state.ColourAttachment ? &colourAttachment : nullptr);
        renderingInfo.pDepthAttachment =(state.DepthAttachment ? &depthAttachment : nullptr);

        vkCmdBeginRendering(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()), &renderingInfo);
    }

    void VulkanRenderer::EndDynamic(Ref<CommandBuffer> cmdBuf)
    {
        VulkanCommandBuffer* vkCmdBuf = HzCast(VulkanCommandBuffer, cmdBuf->Src());

        vkCmdEndRendering(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()));
    }

    void VulkanRenderer::Begin(Ref<CommandBuffer> cmdBuf)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        VulkanCommandBuffer* src = HzCast(VulkanCommandBuffer, cmdBuf->Src());

		uint32_t currentFrame = GetCurrentFrame();
		VkCommandBuffer commandBuffer = src->m_CommandBuffers[currentFrame];

		vkResetFences(context.GetDevice()->GetVkDevice(), 1, &src->m_InFlightFences[currentFrame]);
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    void VulkanRenderer::Begin(Ref<Renderpass> renderpass)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());;

        Ref<CommandBuffer> cmdBuf = renderpass->GetCommandBuffer();
        VulkanRenderpass* src = HzCast(VulkanRenderpass, renderpass->Src());
        VulkanCommandBuffer* cmdSrc = HzCast(VulkanCommandBuffer, cmdBuf->Src());

        Begin(cmdBuf);

        auto size = renderpass->GetSize();
        VkExtent2D extent = { size.first, size.second };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = src->m_RenderPass;
        renderPassInfo.framebuffer = src->m_Framebuffers[context.GetSwapChain()->GetAquiredImage()];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;

        std::vector<VkClearValue> clearValues = {};
        if (!src->m_Specification.ColourAttachment.empty())
        {
            VkClearValue colourClear = {{ { src->m_Specification.ColourClearColour.r, src->m_Specification.ColourClearColour.g, src->m_Specification.ColourClearColour.b, src->m_Specification.ColourClearColour.a } }};
            clearValues.push_back(colourClear);
        }
        if (src->m_Specification.DepthAttachment)
        {
            VkClearValue depthClear = { { { 1.0f, 0 } } };
            clearValues.push_back(depthClear);
        }

        renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmdSrc->m_CommandBuffers[GetCurrentFrame()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)extent.width;
        viewport.height = (float)extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdSrc->m_CommandBuffers[GetCurrentFrame()], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(cmdSrc->m_CommandBuffers[GetCurrentFrame()], 0, 1, &scissor);
    }

    void VulkanRenderer::End(Ref<CommandBuffer> cmdBuf)
    {
        VulkanCommandBuffer* src = HzCast(VulkanCommandBuffer, cmdBuf->Src());

        VK_CHECK_RESULT(vkEndCommandBuffer(src->m_CommandBuffers[GetCurrentFrame()]));
    }

    void VulkanRenderer::End(Ref<Renderpass> renderpass)
    {
        VulkanCommandBuffer* src = HzCast(VulkanCommandBuffer, renderpass->GetCommandBuffer()->Src());
        vkCmdEndRenderPass(src->m_CommandBuffers[GetCurrentFrame()]);

        End(renderpass->GetCommandBuffer());
    }

    void VulkanRenderer::Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        #if defined(HZ_CONFIG_DEBUG)
            VerifyExectionPolicy(policy);
        #endif

        VulkanCommandBuffer* src = HzCast(VulkanCommandBuffer, cmdBuf->Src());

		uint32_t currentFrame = GetCurrentFrame();
		VkDevice device = context.GetDevice()->GetVkDevice();
		VkCommandBuffer commandBuffer = src->m_CommandBuffers[currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::vector<VkSemaphore> semaphores = { };

		for (auto cmd : waitOn)
		{
            VulkanCommandBuffer* vkCmd = HzCast(VulkanCommandBuffer, cmd->Src());
			auto semaphore = vkCmd->m_RenderFinishedSemaphores[currentFrame];

			semaphores.push_back(semaphore);
			m_Manager.Remove(semaphore); // Removes it if it exists
		}

		if (policy & ExecutionPolicy::WaitForPrevious)
		{
            auto semaphore = m_Manager.GetNext();

			// Check if it's not nullptr
			if (semaphore)
				semaphores.push_back(semaphore);
		}

		std::vector<VkPipelineStageFlags> waitStages(semaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT); // TODO: Make customizable?

		submitInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
		submitInfo.pWaitSemaphores = semaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &src->m_RenderFinishedSemaphores[currentFrame];

        // Submission
        switch (queue)
        {
        case Queue::Graphics:
        {
            VK_CHECK_RESULT(vkQueueSubmit(context.GetDevice()->GetGraphicsQueue(), 1, &submitInfo, src->m_InFlightFences[currentFrame]));
            break;
        }
        case Queue::Present:
        {
            VK_CHECK_RESULT(vkQueueSubmit(context.GetDevice()->GetPresentQueue(), 1, &submitInfo, src->m_InFlightFences[currentFrame]));
            break;
        }
        case Queue::Compute:
        {
            VK_CHECK_RESULT(vkQueueSubmit(context.GetDevice()->GetComputeQueue(), 1, &submitInfo, src->m_InFlightFences[currentFrame]));
            break;
        }

        default:
            HZ_LOG_ERROR("Invalid queue selected.");
            break;
        }


		m_Manager.Add(src, policy);
    }

    void VulkanRenderer::Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        Submit(renderpass->GetCommandBuffer(), policy, queue, waitOn);
    }

    void VulkanRenderer::Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount)
    {
        auto vkCmdBuf = HzCast(VulkanCommandBuffer, cmdBuf->Src());
		vkCmdDraw(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()), vertexCount, instanceCount, 0, 0);
    }

    void VulkanRenderer::DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount)
    {
        auto vkCmdBuf = HzCast(VulkanCommandBuffer, cmdBuf->Src());
		vkCmdDrawIndexed(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()), indexBuffer->GetCount(), instanceCount, 0, 0, 0);
    }

    uint32_t VulkanRenderer::GetAcquiredImage() const
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        return context.GetSwapChain()->GetAquiredImage();
    }

    uint32_t VulkanRenderer::GetCurrentFrame() const
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        return context.GetSwapChain()->GetCurrentFrame();
    }

    void VulkanRenderer::VerifyExectionPolicy(ExecutionPolicy& policy) // Should only be used in Debug
    {
        if (!(policy & ExecutionPolicy::InOrder) && !(policy & ExecutionPolicy::Parallel))
        {
            HZ_LOG_WARN("Failed to specify base ExecutionPolicy state. Resorting to ExecutionPolicy::InOrder");
            policy |= ExecutionPolicy::InOrder;
        }
        if (!(policy & ExecutionPolicy::WaitForPrevious) && !(policy & ExecutionPolicy::NoWait))
        {
            HZ_LOG_WARN("Failed to specify extra ExecutionPolicy state. Resorting to ExecutionPolicy::WaitForPrevious");
            policy |= ExecutionPolicy::WaitForPrevious;
        }
    }
}
