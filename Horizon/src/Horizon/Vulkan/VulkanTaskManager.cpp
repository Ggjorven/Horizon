#include "hzpch.h"
#include "VulkanTaskManager.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Utils/Profiler.hpp"

#include <Pulse/Enum/Enum.hpp>

namespace Hz
{

    void VulkanTaskManager::ResetFences()
    {
        HZ_PROFILE_SCOPE("VkTaskManager::ResetFences");
        uint32_t frame = Renderer::GetCurrentFrame();
        m_Fences[frame].clear();
    }

    void VulkanTaskManager::ResetSemaphores()
    {
        HZ_PROFILE_SCOPE("VkTaskManager::ResetSemaphores");
        uint32_t frame = Renderer::GetCurrentFrame();
        m_Semaphores[frame].first.clear();
        m_Semaphores[frame].second.clear();
    }

    void VulkanTaskManager::Add(Ref<VulkanCommandBuffer> cmdBuf, ExecutionPolicy policy)
    {
        HZ_PROFILE_SCOPE("VkTaskManager::Add(CommandBuffer)");
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
        HZ_PROFILE_SCOPE("VkTaskManager::Add(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = Renderer::GetCurrentFrame();
        m_Semaphores[frame].first.push_back(semaphore);
    }

    void VulkanTaskManager::Remove(VkSemaphore semaphore)
    {
        Remove(semaphore, Renderer::GetCurrentFrame());
    }

    void VulkanTaskManager::Remove(VkSemaphore semaphore, uint32_t frame)
    {
        HZ_PROFILE_SCOPE("VkTaskManager::Remove(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

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

    void VulkanTaskManager::RemoveFromAll(VkSemaphore semaphore)
    {
        HZ_PROFILE_SCOPE("VkTaskManager::RemoveFromAll(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        for (uint32_t frame = 0; frame < (uint32_t)Renderer::GetSpecification().Buffers; frame++)
            Remove(semaphore, frame);
    }

    void VulkanTaskManager::Remove(VkFence fence)
    {
        Remove(fence, Renderer::GetCurrentFrame());
    }

    void VulkanTaskManager::Remove(VkFence fence, uint32_t frame)
    {
        HZ_PROFILE_SCOPE("VkTaskManager::Remove(Fence)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        auto it = std::find(m_Fences[frame].begin(), m_Fences[frame].end(), fence);
        if (it != m_Fences[frame].end())
        {
            m_Fences[frame].erase(it);
            return;
        }
    }

    void VulkanTaskManager::RemoveFromAll(VkFence fence)
    {
        HZ_PROFILE_SCOPE("VkTaskManager::RemoveFromAll(Fence)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        for (uint32_t frame = 0; frame < (uint32_t)Renderer::GetSpecification().Buffers; frame++)
            Remove(fence, frame);
    }

    VkSemaphore VulkanTaskManager::GetNext()
    {
        HZ_PROFILE_SCOPE("VkTaskManager::GetNext");
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
