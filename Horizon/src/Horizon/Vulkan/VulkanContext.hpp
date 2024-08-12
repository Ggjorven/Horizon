#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/Image.hpp"

#include "Horizon/Vulkan/VulkanDevice.hpp"
#include "Horizon/Vulkan/VulkanPhysicalDevice.hpp"
#include "Horizon/Vulkan/VulkanSwapChain.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <queue>
#include <mutex>
#include <utility>

namespace Hz
{

	class VulkanContext
	{
	///////////////////////////////////////////////////////////
	// Core functionality
	///////////////////////////////////////////////////////////
	public:
		VulkanContext(void* window);
        ~VulkanContext();

        void Init(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);

        void Free(FreeFunction&& func);
        void FreeObjects();

		inline const VkInstance GetVkInstance() const { return m_VulkanInstance; }
		inline const VkDebugUtilsMessengerEXT GetVkDebugger() const { return m_DebugMessenger; }

		inline Ref<VulkanDevice> GetDevice() const { return m_Device; }
		inline Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline Ref<VulkanSwapChain> GetSwapChain() const { return m_SwapChain; }

        inline std::vector<Ref<Image>>& GetSwapChainImages() { return m_SwapChain->GetSwapChainImages(); }
		inline Ref<Image> GetDepthImage() { return m_SwapChain->GetDepthImage(); }

    ///////////////////////////////////////////////////////////
	// Private functions
	///////////////////////////////////////////////////////////
    private:
        void InitInstance();
        void InitDevices(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);

	///////////////////////////////////////////////////////////
	// Private variables
	///////////////////////////////////////////////////////////
	private:
		VkInstance m_VulkanInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		Ref<VulkanDevice> m_Device = nullptr;
        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
		Ref<VulkanSwapChain> m_SwapChain = nullptr;

        std::mutex m_FreeQueueMutex;
        std::queue<FreeFunction> m_FreeQueue;

        void* m_Window;

	///////////////////////////////////////////////////////////
	// Config variables
	///////////////////////////////////////////////////////////
	public:
		#if !defined(HZ_CONFIG_DIST)
			static constexpr const bool s_Validation = true;
		#else
			static constexpr const bool s_Validation = false;
		#endif

		static const std::vector<const char*> s_RequestedValidationLayers;
		static const std::vector<const char*> s_RequestedDeviceExtensions;

		inline static constinit const std::pair<uint8_t, uint8_t> Version = { 1, 3 }; // Vulkan version 1.3.XXX
	};

}
