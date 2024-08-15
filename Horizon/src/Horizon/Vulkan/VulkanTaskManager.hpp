#pragma once

#include "Horizon/Core/Core.hpp"

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
        ~VulkanTaskManager()
        {

        }

        void ResetFences(); // Resets current frame fences
        void ResetSemaphores(); // Resets current frame semaphores

        void Add(Ref<VulkanCommandBuffer> cmdBuf, ExecutionPolicy policy);
        void Add(VkSemaphore semaphore); // Internal function for swapchain image available semaphore

        void Remove(VkSemaphore semaphore); // It removes the semaphore from current frame if it exists (it checks both semaphore vectors)
        void Remove(VkSemaphore semaphore, uint32_t frame);
        void RemoveFromAll(VkSemaphore semaphore); // It removes the semaphore from all vectors if it exists

        void Remove(VkFence fence); // It removes the fence from current frame if it exists
        void Remove(VkFence fence, uint32_t frame);
        void RemoveFromAll(VkFence fence); // It removes the fence from all vectors if it exists

        VkSemaphore GetNext();

        std::vector<VkFence>& GetFences();
        std::vector<VkSemaphore>& GetSemaphores();

    private:
        std::mutex m_ThreadSafety = {};

        std::unordered_map<uint32_t, std::vector<VkFence>> m_Fences = { };                                              // Waited on by Renderer::EndFrame
        std::unordered_map<uint32_t, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>> m_Semaphores = { }; // Wait on by cmdBufs and finally Renderer::EndFrame
    };

}
