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

#include <Pulse/Core/Defines.hpp>

namespace Hz
{

    void VulkanRenderer::Init(const RendererSpecification& specs)
    {
        s_Data = new Info();
        s_Data->Specification = specs;
    }

    bool VulkanRenderer::Initialized()
    {
        return (s_Data != nullptr);
    }

    void VulkanRenderer::Destroy()
    {
        delete s_Data;
        s_Data = nullptr;
    }

    void VulkanRenderer::Recreate(uint32_t width, uint32_t height, const bool vsync)
    {
        VulkanContext::GetSwapChain()->Init(width, height, vsync, (uint8_t)Renderer::GetSpecification().Buffers);
        s_Data->Specification.VSync = vsync;
    }

    void VulkanRenderer::BeginFrame()
    {
        if (Window::Get().IsMinimized())
            return;

        Renderer::FreeObjects();

        auto swapChain = VulkanContext::GetSwapChain();
        {
            auto& fences = s_Data->Manager.GetFences();
            if (!fences.empty())
            {
                auto device = VulkanContext::GetDevice()->GetVkDevice();

                vkWaitForFences(device, (uint32_t)fences.size(), fences.data(), VK_TRUE, Pulse::Numeric::Max<uint64_t>());
                vkResetFences(device, (uint32_t)fences.size(), fences.data());
            }
            s_Data->Manager.ResetFences();

            s_Data->Manager.Add(swapChain->GetCurrentImageAvailableSemaphore());
        }
        {
            // Acquire SwapChain Image
            swapChain->m_AcquiredImage = swapChain->AcquireNextImage();;
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

        auto& semaphores = s_Data->Manager.GetSemaphores();
        auto swapChain = VulkanContext::GetSwapChain();

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
				vkQueueWaitIdle(VulkanContext::GetDevice()->GetGraphicsQueue());
			}
            #endif

			result = vkQueuePresentKHR(VulkanContext::GetDevice()->GetPresentQueue(), &presentInfo);
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			auto& window = Window::Get();
			Recreate(window.GetWidth(), window.GetHeight(), s_Data->Specification.VSync);
		}
		else if (result != VK_SUCCESS)
		{
			HZ_LOG_ERROR("Failed to present swap chain image!");
		}

        s_Data->Manager.ResetSemaphores();
		swapChain->m_CurrentFrame = (swapChain->m_CurrentFrame + 1) % (uint32_t)s_Data->Specification.Buffers;
    }

    void VulkanRenderer::BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

        VkRenderingAttachmentInfo colourAttachment = {};
        colourAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colourAttachment.imageView = (state.ColourAttachment ? state.ColourAttachment.As<VulkanImage>()->GetVkImageView() : VK_NULL_HANDLE);
        colourAttachment.imageLayout = (state.ColourAttachment ? (VkImageLayout)state.ColourAttachment->GetSpecification().Layout : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colourAttachment.loadOp = (VkAttachmentLoadOp)state.ColourLoadOp;
        colourAttachment.storeOp = (VkAttachmentStoreOp)state.ColourStoreOp;
        colourAttachment.clearValue = { state.ColourClearValue.r, state.ColourClearValue.g, state.ColourClearValue.b, state.ColourClearValue.a };

        VkRenderingAttachmentInfo depthAttachment = {};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = (state.DepthAttachment ? state.DepthAttachment.As<VulkanImage>()->GetVkImageView() : VK_NULL_HANDLE);
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
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

        vkCmdEndRendering(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()));
    }

    void VulkanRenderer::Begin(Ref<CommandBuffer> cmdBuf)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

		uint32_t currentFrame = GetCurrentFrame();
		VkCommandBuffer commandBuffer = vkCmdBuf->m_CommandBuffers[currentFrame];

		vkResetFences(VulkanContext::GetDevice()->GetVkDevice(), 1, &vkCmdBuf->m_InFlightFences[currentFrame]);
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    void VulkanRenderer::Begin(Ref<Renderpass> renderpass)
    {
        Ref<CommandBuffer> cmdBuf = renderpass->GetCommandBuffer();
        Ref<VulkanRenderpass> vkRenderpass = renderpass.As<VulkanRenderpass>();
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

        Begin(cmdBuf);

        auto size = renderpass->GetSize();
        VkExtent2D extent = { size.first, size.second };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vkRenderpass->m_RenderPass;
        renderPassInfo.framebuffer = vkRenderpass->m_Framebuffers[VulkanContext::GetSwapChain()->GetAquiredImage()];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;

        std::vector<VkClearValue> clearValues = {};
        if (!vkRenderpass->m_Specification.ColourAttachment.empty())
        {
            VkClearValue colourClear = {{ { vkRenderpass->m_Specification.ColourClearColour.r, vkRenderpass->m_Specification.ColourClearColour.g, vkRenderpass->m_Specification.ColourClearColour.b, vkRenderpass->m_Specification.ColourClearColour.a } }};
            clearValues.push_back(colourClear);
        }
        if (vkRenderpass->m_Specification.DepthAttachment)
        {
            VkClearValue depthClear = { { { 1.0f, 0 } } };
            clearValues.push_back(depthClear);
        }

        renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(vkCmdBuf->m_CommandBuffers[GetCurrentFrame()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)extent.width;
        viewport.height = (float)extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vkCmdBuf->m_CommandBuffers[GetCurrentFrame()], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(vkCmdBuf->m_CommandBuffers[GetCurrentFrame()], 0, 1, &scissor);
    }

    void VulkanRenderer::End(Ref<CommandBuffer> cmdBuf)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

        VK_CHECK_RESULT(vkEndCommandBuffer(vkCmdBuf->m_CommandBuffers[GetCurrentFrame()]));
    }

    void VulkanRenderer::End(Ref<Renderpass> renderpass)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = renderpass->GetCommandBuffer().As<VulkanCommandBuffer>();
        vkCmdEndRenderPass(vkCmdBuf->m_CommandBuffers[GetCurrentFrame()]);

        End(renderpass->GetCommandBuffer());
    }

    void VulkanRenderer::Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        #if defined(HZ_CONFIG_DEBUG)
            VerifyExectionPolicy(policy);
        #endif

        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

		uint32_t currentFrame = GetCurrentFrame();
		VkCommandBuffer commandBuffer = vkCmdBuf->m_CommandBuffers[currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::vector<VkSemaphore> semaphores = { };

		for (auto cmd : waitOn)
		{
            Ref<VulkanCommandBuffer> vkCmd = cmd.As<VulkanCommandBuffer>();
			auto semaphore = vkCmd->m_RenderFinishedSemaphores[currentFrame];

			semaphores.push_back(semaphore);
			s_Data->Manager.Remove(semaphore); // Removes it if it exists
		}

		if (policy & ExecutionPolicy::WaitForPrevious)
		{
            auto semaphore = s_Data->Manager.GetNext();

			// Check if it's not nullptr
			if (semaphore)
				semaphores.push_back(semaphore);
		}

		std::vector<VkPipelineStageFlags> waitStages(semaphores.size(), (VkPipelineStageFlagBits)waitStage);

		submitInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
		submitInfo.pWaitSemaphores = semaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vkCmdBuf->m_RenderFinishedSemaphores[currentFrame];

        // Submission
        switch (queue)
        {
        case Queue::Graphics:
        {
            VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::GetDevice()->GetGraphicsQueue(), 1, &submitInfo, vkCmdBuf->m_InFlightFences[currentFrame]));
            break;
        }
        case Queue::Present:
        {
            VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::GetDevice()->GetPresentQueue(), 1, &submitInfo, vkCmdBuf->m_InFlightFences[currentFrame]));
            break;
        }
        case Queue::Compute:
        {
            VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::GetDevice()->GetComputeQueue(), 1, &submitInfo, vkCmdBuf->m_InFlightFences[currentFrame]));
            break;
        }

        default:
            HZ_LOG_ERROR("Invalid queue selected.");
            break;
        }

		s_Data->Manager.Add(vkCmdBuf, policy);
    }

    void VulkanRenderer::Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<Ref<CommandBuffer>>& waitOn)
    {
        Submit(renderpass->GetCommandBuffer(), policy, queue, waitStage, waitOn);
    }

    void VulkanRenderer::Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount, uint32_t instanceCount)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

		vkCmdDraw(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()), vertexCount, instanceCount, 0, 0);
    }

    void VulkanRenderer::DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount)
    {
        Ref<VulkanCommandBuffer> vkCmdBuf = cmdBuf.As<VulkanCommandBuffer>();

		vkCmdDrawIndexed(vkCmdBuf->GetVkCommandBuffer(GetCurrentFrame()), indexBuffer->GetCount(), instanceCount, 0, 0, 0);
    }

    void VulkanRenderer::Free(FreeFunction&& func)
    {
        std::scoped_lock<std::mutex> lock(s_FreeQueueMutex);
        s_FreeQueue.push(std::move(func));
    }

    void VulkanRenderer::FreeObjects()
    {
        if (s_FreeQueue.empty()) return;

        VulkanContext::GetDevice()->Wait(); // Wait till idle

        // We repeat this, because sometimes the function calls Free() of another objects and that will be unresolved without repeating
        while (!s_FreeQueue.empty())
        {
            std::queue<FreeFunction> functions = {};
            functions.swap(s_FreeQueue);

            while (!functions.empty())
            {
                auto func = functions.front();
                func();
                functions.pop();
            }
        }
    }

    uint32_t VulkanRenderer::GetAcquiredImage()
    {
        return VulkanContext::GetSwapChain()->GetAquiredImage();
    }

    uint32_t VulkanRenderer::GetCurrentFrame()
    {
        return VulkanContext::GetSwapChain()->GetCurrentFrame();
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
