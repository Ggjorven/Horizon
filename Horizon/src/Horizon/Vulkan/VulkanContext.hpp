#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
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
        static void Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);
        static void Destroy();

		inline static const VkInstance GetVkInstance() { return s_Data->VulkanInstance; }
		inline static const VkDebugUtilsMessengerEXT GetVkDebugger() { return s_Data->DebugMessenger; }

		inline static Ref<VulkanDevice> GetDevice() { return s_Data->Device; }
		inline static Ref<VulkanPhysicalDevice> GetPhysicalDevice() { return s_Data->PhysicalDevice; }
		inline static Ref<VulkanSwapChain> GetSwapChain() { return s_Data->SwapChain; }

        inline static std::vector<Ref<Image>>& GetSwapChainImages() { return s_Data->SwapChain->GetSwapChainImages(); }
		inline static Ref<Image> GetDepthImage() { return s_Data->SwapChain->GetDepthImage(); }

    ///////////////////////////////////////////////////////////
	// Private functions
	///////////////////////////////////////////////////////////
    private:
        static void InitInstance();
        static void InitDevices(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);

	///////////////////////////////////////////////////////////
	// Private variables
	///////////////////////////////////////////////////////////
	private:
        // Note: We store our info in a struct, so we can ensure lifetime
        // of all objects easily while the class remains static.
        struct Info
        {
        public:
            VkInstance VulkanInstance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

            Ref<VulkanDevice> Device = nullptr;
            Ref<VulkanPhysicalDevice> PhysicalDevice = nullptr;
            Ref<VulkanSwapChain> SwapChain = nullptr;

            void* Window = nullptr;
        };

        inline static Info* s_Data = nullptr;

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
