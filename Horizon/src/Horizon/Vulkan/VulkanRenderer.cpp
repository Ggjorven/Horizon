#include "hzpch.h"
#include "VulkanRenderer.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

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
        ENFORCE_API(Vulkan);

        VulkanContext& context = *GraphicsContext::Raw();

        context.GetSwapChain()->Init(width, height, vsync, (uint8_t)Renderer::GetSpecification().Buffers);
        m_Specification.VSync = vsync;
    }

    void VulkanRenderer::BeginFrame()
    {
        ENFORCE_API(Vulkan);

        if (Window::Get().IsMinimized())
            return;

        VulkanContext& context = *GraphicsContext::Raw();
        auto swapChain = context.GetSwapChain();

        {
            auto& fences = m_Manager.GetFences();
            if (!fences.empty())
            {
                vkWaitForFences(context.GetDevice()->GetVkDevice(), (uint32_t)fences.size(), fences.data(), VK_TRUE, 18446744073709551615ull);
                vkResetFences(context.GetDevice()->GetVkDevice(), (uint32_t)fences.size(), fences.data());
            }

            m_Manager.Add(context.GetSwapChain()->GetCurrentImageAvailableSemaphore());
        }
        { // Acquire SwapChain Image
            uint32_t acquiredImage = swapChain->AcquireNextImage();
            swapChain->m_AcquiredImage = acquiredImage;
        }
    }

    void VulkanRenderer::EndFrame()
    {
        if (Window::Get().IsMinimized())
            return;

        // Note: This function is empty, because there is nothing to be done
    }

    void VulkanRenderer::Present()
    {
        ENFORCE_API(Vulkan);

        if (Window::Get().IsMinimized())
            return;

        VulkanContext& context = *GraphicsContext::Raw();

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
			// Note(Jorben): Without these 2 lines there is a memory leak when validation layers are enabled.
			if constexpr (VulkanContext::s_Validation)
			{
				vkQueueWaitIdle(context.GetDevice()->GetGraphicsQueue());
			}

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

        m_Manager.Reset();
		swapChain->m_CurrentFrame = (swapChain->m_CurrentFrame + 1) % (uint32_t)m_Specification.Buffers;
    }

    uint32_t VulkanRenderer::GetCurrentFrame() const
    {
        ENFORCE_API(Vulkan, -1);

        const VulkanContext& context = *GraphicsContext::Raw();

        return context.GetSwapChain()->GetCurrentFrame();
    }

    void VulkanRenderer::VerifyExectionPolicy(ExecutionPolicy &policy) // Only used in Debug
    {
        if (!(policy & ExecutionPolicy::InOrder) || !(policy & ExecutionPolicy::Parallel))
        {
            HZ_LOG_WARN("Failed to specify base ExecutionPolicy state. Resorting to ExecutionPolicy::InOrder");
            policy |= ExecutionPolicy::InOrder;
        }
        if (!(policy & ExecutionPolicy::WaitForPrevious) || !(policy & ExecutionPolicy::NoWait))
        {
            HZ_LOG_WARN("Failed to specify extra ExecutionPolicy state. Resorting to ExecutionPolicy::WaitForPrevious");
            policy |= ExecutionPolicy::WaitForPrevious;
        }
    }
}
