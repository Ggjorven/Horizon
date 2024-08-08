#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

#include <vulkan/vulkan.h>

#include <span>
#include <mutex>
#include <array>
#include <vector>
#include <ranges>
#include <utility>

namespace Hz
{

    class VulkanTaskManager
    {
    public:
        VulkanTaskManager() = default;
        ~VulkanTaskManager() = default;

        void Reset(); // Resets current frame containers

        void Add(VulkanCommandBuffer* cmdBuf, ExecutionPolicy policy);
        void Add(VkSemaphore semaphore); // Adds it to the frame wait queue (m_Semaphores[frame][1])

        VkSemaphore GetNext();

        std::vector<VkFence>& GetFences();
        std::vector<VkSemaphore>& GetSemaphores();

    private:
        std::mutex m_ThreadSafety = {};

        std::unordered_map<uint32_t, std::vector<VkFence>> m_Fences = { };                                              // Waited on by Renderer::EndFrame
        std::unordered_map<uint32_t, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>> m_Semaphores = { }; // Wait on by cmdBufs and finally Renderer::EndFrame
    };

}
