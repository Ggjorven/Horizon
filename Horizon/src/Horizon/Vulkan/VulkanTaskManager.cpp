#include "hzpch.h"
#include "VulkanTaskManager.hpp"

#include "Horizon/Core/Logging.hpp"

namespace Hz
{

    void VulkanTaskManager::Reset()
    {
        uint32_t frame = Renderer::GetCurrentFrame();

        m_Fences[frame].clear();
        m_Semaphores[frame].first.clear();
        m_Semaphores[frame].second.clear();
    }

    void VulkanTaskManager::Add(VulkanCommandBuffer* cmdBuf, ExecutionPolicy policy)
    {
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = Renderer::GetCurrentFrame();
        m_Fences[frame].emplace_back(cmdBuf->GetVkInFlightFence(frame));

        if (policy & ExecutionPolicy::InOrder)
            m_Semaphores[frame].first.push_back(cmdBuf->GetVkRenderFinishedSemaphore(frame));
        else if (policy & ExecutionPolicy::Parallel)
            m_Semaphores[frame].second.push_back(cmdBuf->GetVkRenderFinishedSemaphore(frame));
    }

    void VulkanTaskManager::Add(VkSemaphore semaphore)
    {
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = Renderer::GetCurrentFrame();
        m_Semaphores[frame].second.push_back(semaphore);
    }

    void VulkanTaskManager::Remove(VkSemaphore semaphore)
    {
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = Renderer::GetCurrentFrame();

        // Check the InOrder list
        auto it = std::find(m_Semaphores[frame].first.begin(), m_Semaphores[frame].first.end(), semaphore);
        if (it != m_Semaphores[frame].first.end())
        {
            m_Semaphores[frame].first.erase(it);
            return;
        }

        // Check the Frame End list
        auto it2 = std::find(m_Semaphores[frame].second.begin(), m_Semaphores[frame].second.end(), semaphore);
        if (it2 != m_Semaphores[frame].second.end())
        {
            m_Semaphores[frame].second.erase(it2);
            return;
        }
    }

    VkSemaphore VulkanTaskManager::GetNext()
    {
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = Renderer::GetCurrentFrame();
        if (m_Semaphores[frame].first.empty())
            return VK_NULL_HANDLE;

        VkSemaphore semaphore = m_Semaphores[frame].first[0];
        m_Semaphores[frame].first.erase(m_Semaphores[frame].first.begin() + 0);

        return semaphore;
    }

    std::vector<VkFence>& VulkanTaskManager::GetFences()
    {
        return m_Fences[Renderer::GetCurrentFrame()];
    }

    std::vector<VkSemaphore>& VulkanTaskManager::GetSemaphores()
    {
        static std::vector<VkSemaphore> semaphores = {};
        semaphores.clear();

        uint32_t frame = Renderer::GetCurrentFrame();

        semaphores.reserve(m_Semaphores[frame].first.size() + m_Semaphores[frame].second.size()); // Reserve space to avoid multiple allocations
        semaphores.insert(semaphores.end(), m_Semaphores[frame].first.begin(), m_Semaphores[frame].first.end());
        semaphores.insert(semaphores.end(), m_Semaphores[frame].second.begin(), m_Semaphores[frame].second.end());

        return semaphores;
    }

}
