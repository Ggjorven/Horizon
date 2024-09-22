#include "hzpch.h"
#include "VulkanSwapChain.hpp"

#include "Horizon/IO/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanImage.hpp"
#include "Horizon/Vulkan/VulkanPhysicalDevice.hpp"

#include <Pulse/Core/Defines.hpp>
#include <Pulse/Enum/Enum.hpp>

namespace Hz
{

	VulkanSwapChain::VulkanSwapChain(VkSurfaceKHR surface)
		: m_Surface(surface)
	{
        FindImageFormatAndColorSpace();

		///////////////////////////////////////////////////////////
		// Command pools
		///////////////////////////////////////////////////////////
		if (!m_CommandPool)
		{
			QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::Find(m_Surface, VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice());

			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows us to reset the command buffer and reuse it.
			poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

			VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::GetDevice()->GetVkDevice(), &poolInfo, nullptr, &m_CommandPool));
		}
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
        auto device = VulkanContext::GetDevice()->GetVkDevice();
        VulkanContext::GetDevice()->Wait();

        vkQueueWaitIdle(VulkanContext::GetDevice()->GetGraphicsQueue());

		if (m_SwapChain)
			vkDestroySwapchainKHR(device, m_SwapChain, nullptr);

		m_Images.clear();
		m_DepthStencil.Reset();

		vkDestroyCommandPool(device, m_CommandPool, nullptr);

		for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);

        vkDestroySurfaceKHR(VulkanContext::GetVkInstance(), m_Surface, nullptr);
	}

	void VulkanSwapChain::Init(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight)
	{
		///////////////////////////////////////////////////////////
		// SwapChain
		///////////////////////////////////////////////////////////
		SwapChainSupportDetails details = SwapChainSupportDetails::Query(m_Surface, VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice());

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (details.Capabilities.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = details.Capabilities.currentExtent;
		}

		// Set the pretransform (used for mobile rotation handling)
		m_PreTransform = details.Capabilities.currentTransform;
		if (m_PreTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || m_PreTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
		{
			std::swap(swapchainExtent.width, swapchainExtent.height);
			Window::Get().SwapWidthAndHeight();
		}

		if (width == 0 || height == 0)
			return;

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (size_t i = 0; i < details.PresentModes.size(); i++)
			{
				if (details.PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (details.PresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = details.Capabilities.minImageCount + 1;
		if ((details.Capabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > details.Capabilities.maxImageCount))
		{
			desiredNumberOfSwapchainImages = details.Capabilities.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (details.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = details.Capabilities.currentTransform;

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {

			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (details.Capabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = NULL;
		swapchainCI.surface = m_Surface;
		swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCI.imageFormat = m_ColourFormat;
		swapchainCI.imageColorSpace = m_ColourSpace;
		swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };

		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (details.Capabilities.supportedTransforms & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = NULL;
		swapchainCI.presentMode = swapchainPresentMode;
		swapchainCI.oldSwapchain = m_SwapChain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchainCI.clipped = VK_TRUE;
		swapchainCI.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Enable transfer destination on swap chain images if supported
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        auto device = VulkanContext::GetDevice()->GetVkDevice();
		auto oldSwapchain = m_SwapChain;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_SwapChain));

		if (oldSwapchain)
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr); // Destroys images?

		// Get the swap chain images
		uint32_t imageCount = 0;
		std::vector<VkImage> tempImages = { };

		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, NULL));
		tempImages.resize(imageCount);
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, tempImages.data()));

		if (m_Images.empty()) m_Images.resize((size_t)imageCount); // Make sure we can access the appropriate indices
		for (uint32_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = m_ColourFormat;
			colorAttachmentView.image = tempImages[i];
			colorAttachmentView.components =
			{
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

            VkImageView imageView = VK_NULL_HANDLE;
            VK_CHECK_RESULT(vkCreateImageView(device, &colorAttachmentView, nullptr, &imageView));

			ImageSpecification specs = {};
			specs.Usage = ImageUsage::None;
			specs.Format = (ImageFormat)m_ColourFormat;
			specs.Flags = ImageUsageFlags::Colour;
			specs.Width = width;
			specs.Height = height;
			specs.Layout = ImageLayout::Undefined;
            specs.MipMaps = false;

            if (m_Images[i])
            {
                Ref<VulkanImage> src = m_Images[i].As<VulkanImage>();

                // Destroy old image view
				Renderer::Free([device = device, imageView = src->m_ImageView]()
				{
					vkDestroyImageView(device, imageView, nullptr);
				});

                // Set new data
                src->m_Specification = specs;
                src->m_Image = tempImages[i];
                src->m_ImageView = imageView;
            }
            else
                m_Images[i] = Ref<VulkanImage>::Create(specs, tempImages[i], imageView);

            m_Images[i]->Transition(ImageLayout::Undefined, ImageLayout::PresentSrcKHR);
		}

		if (!m_DepthStencil)
		{
			ImageSpecification specs = {};
			specs.Usage = ImageUsage::Size;
			specs.Format = (ImageFormat)VkUtils::FindDepthFormat();
			specs.Flags = ImageUsageFlags::DepthStencil | ImageUsageFlags::Sampled;
			specs.Width = width;
			specs.Height = height;
			specs.Layout = ImageLayout::DepthStencil;
            specs.MipMaps = false;

			m_DepthStencil = Image::Create(specs);
		}
		else
			m_DepthStencil->Resize(width, height);

        ///////////////////////////////////////////////////////////
		// Synchronization Objects
        ///////////////////////////////////////////////////////////
		if (m_ImageAvailableSemaphores.empty())
		{
			m_ImageAvailableSemaphores.resize((size_t)framesInFlight);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < (size_t)framesInFlight; i++)
			{
                VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			}
		}
	}

    Ref<VulkanSwapChain> VulkanSwapChain::Create(VkSurfaceKHR surface)
    {
        return Ref<VulkanSwapChain>::Create(surface);
    }

    uint32_t VulkanSwapChain::AcquireNextImage()
	{
		uint32_t imageIndex = 0;

		VkResult result = vkAcquireNextImageKHR(VulkanContext::GetDevice()->GetVkDevice(), m_SwapChain, Pulse::Numeric::Max<uint64_t>(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			auto& window = Window::Get();
			Init(window.GetWidth(), window.GetHeight(), Renderer::GetSpecification().VSync, (uint8_t)Renderer::GetSpecification().Buffers);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			HZ_LOG_ERROR("Failed to acquire SwapChain image!");
		}

		return imageIndex;
	}

	void VulkanSwapChain::FindImageFormatAndColorSpace()
    {
        const VkPhysicalDevice physicalDevice = VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice();
		const VkSurfaceKHR surface = m_Surface;

		// Get list of supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_ColourFormat = VK_FORMAT_B8G8R8A8_UNORM;
			m_ColourSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_ColourFormat = surfaceFormat.format;
					m_ColourSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_ColourFormat = surfaceFormats[0].format;
				m_ColourSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

	bool VulkanSwapChain::Is180Rotation() const
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice(), VulkanContext::GetSwapChain()->GetVkSurface(), &surfaceCapabilities));

		VkSurfaceTransformFlagsKHR currentTransform = surfaceCapabilities.currentTransform;
		switch (currentTransform)
		{
		case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:		return m_PreTransform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR;
		case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:	return m_PreTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR;
		case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:	return m_PreTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:	return m_PreTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR;

		default:
			break;
		}

		return false;
	}

}
