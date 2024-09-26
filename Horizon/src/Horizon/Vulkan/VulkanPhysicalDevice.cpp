#include "hzpch.h"
#include "VulkanPhysicalDevice.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz
{

	VulkanPhysicalDevice::VulkanPhysicalDevice(const VkSurfaceKHR surface)
	{
		const VkInstance instance = VulkanContext::GetVkInstance();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		HZ_ASSERT(!(deviceCount == 0), "Failed to find a GPU with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (PhysicalDeviceSuitable(surface, device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}
		HZ_ASSERT(m_PhysicalDevice, "Failed to find a GPU with support for this application's required Vulkan capabilities!");

		m_DepthFormat = GetDepthFormat();
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
		// Note:  Since a Physical Device is not something we created there is nothing to destroy
	}

	QueueFamilyIndices QueueFamilyIndices::Find(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = {};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// Early exit check
			if (indices.IsComplete())
				break;

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.GraphicsFamily = i;

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.ComputeFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.PresentFamily = i;

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails SwapChainSupportDetails::Query(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize((size_t)formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
		}

		// Presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize((size_t)presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	bool VulkanPhysicalDevice::PhysicalDeviceSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = QueueFamilyIndices::Find(surface, device);

		bool extensionsSupported = ExtensionsSupported(device);
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::Query(surface, device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		// Index features
		VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT, nullptr };
		VkPhysicalDeviceFeatures2 deviceFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexFeatures };

		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

		// Check support for bindless textures
		bool bindlessSupport = indexFeatures.descriptorBindingPartiallyBound && indexFeatures.runtimeDescriptorArray;

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && FeaturesSupported(VulkanContext::s_RequestedDeviceFeatures, supportedFeatures) && bindlessSupport;
	}

	bool VulkanPhysicalDevice::ExtensionsSupported(const VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(VulkanContext::s_RequestedDeviceExtensions.begin(), VulkanContext::s_RequestedDeviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// Note:  It's empty if all the required extensions are available
		return requiredExtensions.empty();
	}

	VkFormat VulkanPhysicalDevice::GetDepthFormat()
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		std::vector<VkFormat> depthFormats =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);

			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return format;
		}

		return VK_FORMAT_UNDEFINED;
	}

	Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Select(const VkSurfaceKHR surface)
	{
		return Ref<VulkanPhysicalDevice>::Create(surface);
	}

	bool VulkanPhysicalDevice::FeaturesSupported(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& found)
    {
        bool failed = false;

        #define FEATURE_TEST(feature) failed |= (requested.feature ? (found.feature == (VkBool32)false) : false)
        
        FEATURE_TEST(robustBufferAccess);
        FEATURE_TEST(robustBufferAccess);
        FEATURE_TEST(fullDrawIndexUint32);
        FEATURE_TEST(imageCubeArray);
        FEATURE_TEST(independentBlend);
        FEATURE_TEST(geometryShader);
        FEATURE_TEST(tessellationShader);
        FEATURE_TEST(sampleRateShading);
        FEATURE_TEST(dualSrcBlend);
        FEATURE_TEST(logicOp);
        FEATURE_TEST(multiDrawIndirect);
        FEATURE_TEST(drawIndirectFirstInstance);
        FEATURE_TEST(depthClamp);
        FEATURE_TEST(depthBiasClamp);
        FEATURE_TEST(fillModeNonSolid);
        FEATURE_TEST(depthBounds);
        FEATURE_TEST(wideLines);
        FEATURE_TEST(largePoints);
        FEATURE_TEST(alphaToOne);
        FEATURE_TEST(multiViewport);
        FEATURE_TEST(samplerAnisotropy);
        FEATURE_TEST(textureCompressionETC2);
        FEATURE_TEST(textureCompressionASTC_LDR);
        FEATURE_TEST(textureCompressionBC);
        FEATURE_TEST(occlusionQueryPrecise);
        FEATURE_TEST(pipelineStatisticsQuery);
        FEATURE_TEST(vertexPipelineStoresAndAtomics);
        FEATURE_TEST(fragmentStoresAndAtomics);
        FEATURE_TEST(shaderTessellationAndGeometryPointSize);
        FEATURE_TEST(shaderImageGatherExtended);
        FEATURE_TEST(shaderStorageImageExtendedFormats);
        FEATURE_TEST(shaderStorageImageMultisample);
        FEATURE_TEST(shaderStorageImageReadWithoutFormat);
        FEATURE_TEST(shaderStorageImageWriteWithoutFormat);
        FEATURE_TEST(shaderUniformBufferArrayDynamicIndexing);
        FEATURE_TEST(shaderSampledImageArrayDynamicIndexing);
        FEATURE_TEST(shaderStorageBufferArrayDynamicIndexing);
        FEATURE_TEST(shaderStorageImageArrayDynamicIndexing);
        FEATURE_TEST(shaderClipDistance);
        FEATURE_TEST(shaderCullDistance);
        FEATURE_TEST(shaderFloat64);
        FEATURE_TEST(shaderInt64);
        FEATURE_TEST(shaderInt16);
        FEATURE_TEST(shaderResourceResidency);
        FEATURE_TEST(shaderResourceMinLod);
        FEATURE_TEST(sparseBinding);
        FEATURE_TEST(sparseResidencyBuffer);
        FEATURE_TEST(sparseResidencyImage2D);
        FEATURE_TEST(sparseResidencyImage3D);
        FEATURE_TEST(sparseResidency2Samples);
        FEATURE_TEST(sparseResidency4Samples);
        FEATURE_TEST(sparseResidency8Samples);
        FEATURE_TEST(sparseResidency16Samples);
        FEATURE_TEST(sparseResidencyAliased);
        FEATURE_TEST(variableMultisampleRate);
        FEATURE_TEST(inheritedQueries);

        return !failed;
    }

}
