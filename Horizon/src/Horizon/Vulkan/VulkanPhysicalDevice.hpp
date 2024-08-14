#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <optional>

#include <vulkan/vulkan.h>

namespace Hz
{

	struct QueueFamilyIndices
	{
	public:
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> ComputeFamily;
		std::optional<uint32_t> PresentFamily;

		static QueueFamilyIndices Find(const VkSurfaceKHR surface, const VkPhysicalDevice device);

	public:
		inline bool IsComplete() const { return GraphicsFamily.has_value() && ComputeFamily.has_value() && PresentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
	public:
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;

		static SwapChainSupportDetails Query(const VkSurfaceKHR surface, const VkPhysicalDevice device);
	};



	class VulkanPhysicalDevice : public RefCounted
	{
	public:
		VulkanPhysicalDevice(const VkSurfaceKHR surface);
		virtual ~VulkanPhysicalDevice();

		inline const VkFormat GetDepthFormat() const { return m_DepthFormat; }
		inline const VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }

		static Ref<VulkanPhysicalDevice> Select(const VkSurfaceKHR surface);

	private:
		bool PhysicalDeviceSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice device);
		bool ExtensionsSupported(const VkPhysicalDevice device);
		VkFormat GetDepthFormat();

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
	};

}
