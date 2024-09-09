#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Image.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <utility>

namespace Hz
{

    class VulkanRenderer;

	class VulkanSwapChain : public RefCounted
	{
	public:
		VulkanSwapChain(VkSurfaceKHR surface); // Takes ownership of the surface
		~VulkanSwapChain();

		void Init(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight);

		inline const VkFormat GetColourFormat() const { return m_ColourFormat; }

		inline uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline uint32_t GetAquiredImage() const { return m_AcquiredImage; }

		inline std::vector<Ref<Image>>& GetSwapChainImages() { return m_Images; }
		inline Ref<Image> GetDepthImage() { return m_DepthStencil; }

		inline const VkSurfaceKHR GetVkSurface() const { return m_Surface; }
		inline const VkCommandPool GetVkCommandPool() const { return m_CommandPool; }
		inline const VkSemaphore GetImageAvailableSemaphore(uint32_t index) const { return m_ImageAvailableSemaphores[index]; }
		inline const VkSemaphore GetCurrentImageAvailableSemaphore() const { return GetImageAvailableSemaphore(m_CurrentFrame); }

        static Ref<VulkanSwapChain> Create(VkSurfaceKHR surface); // Takes ownership of the surface

	private:
		uint32_t AcquireNextImage();
		void FindImageFormatAndColorSpace();

		bool Is180Rotation();

	private:
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface;

		std::vector<Ref<Image>> m_Images = { };
		Ref<Image> m_DepthStencil = nullptr;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores = { };

		VkFormat m_ColourFormat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR m_ColourSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
		VkSurfaceTransformFlagBitsKHR m_PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		uint32_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;

        friend class VulkanRenderer;
	};

}
