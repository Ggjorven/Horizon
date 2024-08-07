#include "hzpch.h"
#include "VulkanRenderer.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

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

        context.GetSwapChain()->Init(width, height, vsync);
        m_Specification.VSync = vsync;
    }

    void VulkanRenderer::BeginFrame()
    {
        ENFORCE_API(Vulkan);

        if (Window::Get().IsMinimized())
            return;

        VulkanContext& context = *GraphicsContext::Raw();

        {
            /*
            Renderer::GetRenderData().Reset();
            m_ResourceFreeQueue.Execute();

            auto& fences = VulkanTaskManager::GetFences();
            if (!fences.empty())
            {
                vkWaitForFences(m_Device->GetVulkanDevice(), (uint32_t)fences.size(), fences.data(), VK_TRUE, MAX_UINT64);
                vkResetFences(m_Device->GetVulkanDevice(), (uint32_t)fences.size(), fences.data());
            }
            VulkanTaskManager::AddSemaphore(m_SwapChain->GetCurrentImageAvailableSemaphore());
            */
        }
        { // Acquire SwapChain Image
            uint32_t acquiredImage = context.GetSwapChain()->AcquireNextImage();
            context.GetSwapChain()->m_AcquiredImage = acquiredImage;
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
        if (Window::Get().IsMinimized())
            return;

        /**
        auto& semaphores = VulkanTaskManager::GetSemaphores();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
		presentInfo.pWaitSemaphores = semaphores.data();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &m_AquiredImage;
		presentInfo.pResults = nullptr; // Optional

		VkResult result = VK_SUCCESS;
		{
			APP_PROFILE_SCOPE("QueuePresent");

			// Note(Jorben): Without these 2 lines there is a memory leak when validation layers are enabled.
			if constexpr (s_Validation)
			{
				APP_PROFILE_SCOPE("QueueWaitIdle");
				vkQueueWaitIdle(m_Device->GetGraphicsQueue());
			}

			result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo);
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			auto& window = Application::Get().GetWindow();
			OnResize(window.GetWidth(), window.GetHeight(), window.IsVSync());
		}
		else if (result != VK_SUCCESS)
		{
			APP_LOG_ERROR("Failed to present swap chain image!");
		}

		constexpr const uint32_t framesInFlight = (uint32_t)RendererSpecification::BufferCount;
		m_CurrentFrame = (m_CurrentFrame + 1) % framesInFlight;
        */
    }

}
