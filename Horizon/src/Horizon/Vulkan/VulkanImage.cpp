#include "hzpch.h"
#include "VulkanImage.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanDescriptors.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <stb_image.h>

namespace Hz
{

    static VkImageAspectFlags GetVulkanImageAspectFromImageUsage(ImageUsageFlags usage);

	VulkanImage::VulkanImage(const ImageSpecification& specs, const SamplerSpecification& samplerSpecs)
		: m_Specification(specs), m_SamplerSpecification(samplerSpecs)
	{
        HZ_ASSERT(((m_Specification.Flags & ImageUsageFlags::Colour) || (m_Specification.Flags & ImageUsageFlags::DepthStencil)), "Tried to create image without specifying if it's a Colour or Depth image.")

        switch (m_Specification.Usage)
		{
		case ImageUsage::Size:
			CreateImage(m_Specification.Width, m_Specification.Height);
			break;
		case ImageUsage::File:
			CreateImage(m_Specification.Path);
			break;

		default:
			HZ_LOG_ERROR("Invalid image usage selected.");
			break;
		}
    }

    VulkanImage::VulkanImage(const ImageSpecification& specs, const VkImage image, const VkImageView imageView)
        : m_Specification(specs), m_SamplerSpecification({}), m_Image(image), m_ImageView(imageView) // For SwapChain
    {
    }

	VulkanImage::~VulkanImage()
	{
        Destroy();
	}

	void VulkanImage::SetData(void* data, size_t size)
	{
        ImageLayout desiredLayout = m_Specification.Layout;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
		stagingBufferAllocation = VkUtils::Allocator::AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* mappedData;
		VkUtils::Allocator::MapMemory(stagingBufferAllocation, mappedData);
		memcpy(mappedData, data, size);
		VkUtils::Allocator::UnMapMemory(stagingBufferAllocation);

        Transition(ImageLayout::Undefined, ImageLayout::TransferDst);
		VkUtils::Allocator::CopyBufferToImage(stagingBuffer, m_Image, m_Specification.Width, m_Specification.Height);

		if (m_Specification.MipMaps)
		{
			GenerateMipmaps(m_Image, (VkFormat)m_Specification.Format, m_Specification.Width, m_Specification.Height, m_Miplevels);
			Transition(ImageLayout::ShaderRead, desiredLayout);
		}
		else
		{
            Transition(ImageLayout::TransferDst, desiredLayout);
		}

		VkUtils::Allocator::DestroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void VulkanImage::Resize(uint32_t width, uint32_t height)
	{
        Destroy();
		CreateImage(width, height);
	}

    void VulkanImage::Transition(ImageLayout initial, ImageLayout final)
    {
        if (initial == final) return;

		VulkanCommand command = VulkanCommand(true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = (VkImageLayout)initial;
		barrier.newLayout = (VkImageLayout)final;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = m_Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_Miplevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage = {};
		VkPipelineStageFlags destinationStage = {};

		// Aspect checks
		if ((VkImageLayout)final == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || (VkImageLayout)initial == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			|| (VkImageLayout)final == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || (VkImageLayout)initial == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // Check if it has stencil component
			if ((VkFormat)m_Specification.Format == VK_FORMAT_D32_SFLOAT_S8_UINT || (VkFormat)m_Specification.Format == VK_FORMAT_D24_UNORM_S8_UINT)
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        using namespace Pulse;

        switch (Enum::Fuse((VkImageLayout)initial, (VkImageLayout)final))
        {
        // Undefined
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL):
        {
            barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        }
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL):
        {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        }
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
		}

        // Colour attachment
        case Enum::Fuse(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
		}

        // Depth Stencil Attachments
        case Enum::Fuse(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
		}

        // Depth Stencil Read
        case Enum::Fuse(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
		}

        // Transfer Src
        case Enum::Fuse(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
		}

        // Transfer Dst
        case Enum::Fuse(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
		}

        // Shader Read
        case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
		}
        case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
		}

        // Present Src
        case Enum::Fuse(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
		}

        default:
            HZ_ASSERT(false, "Layout transition not supported.");
            break;
        }

        vkCmdPipelineBarrier(command.GetVkCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		command.EndAndSubmit();
	}

    void VulkanImage::CreateImage(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		if (m_Specification.MipMaps)
			m_Miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		m_Allocation = VkUtils::Allocator::AllocateImage(width, height, m_Miplevels, (VkFormat)m_Specification.Format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | (VkImageUsageFlagBits)m_Specification.Flags, VMA_MEMORY_USAGE_GPU_ONLY, m_Image);

		m_ImageView = VkUtils::Allocator::CreateImageView(m_Image, (VkFormat)m_Specification.Format, GetVulkanImageAspectFromImageUsage(m_Specification.Flags), m_Miplevels);
		m_Sampler = VkUtils::Allocator::CreateSampler((VkFilter)m_SamplerSpecification.MagFilter, (VkFilter)m_SamplerSpecification.MinFilter, (VkSamplerAddressMode)m_SamplerSpecification.Address, (VkSamplerMipmapMode)m_SamplerSpecification.Mipmaps, m_Miplevels);

		Transition(ImageLayout::Undefined, m_Specification.Layout);
	}

	void VulkanImage::CreateImage(const std::filesystem::path& path)
	{
		int width, height, texChannels;
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);

        HZ_ASSERT((pixels != nullptr), "Failed to load image from '{0}'", path.string());

		m_Specification.Width = width;
		m_Specification.Height = height;
		if (m_Specification.MipMaps)
			m_Miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		m_Specification.Format = ImageFormat::RGBA;
		size_t imageSize = m_Specification.Width * m_Specification.Height * 4;

		m_Allocation = VkUtils::Allocator::AllocateImage(m_Specification.Width, m_Specification.Height, m_Miplevels, (VkFormat)m_Specification.Format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | (VkImageUsageFlagBits)m_Specification.Flags, VMA_MEMORY_USAGE_GPU_ONLY, m_Image);

		m_ImageView = VkUtils::Allocator::CreateImageView(m_Image, (VkFormat)m_Specification.Format, VK_IMAGE_ASPECT_COLOR_BIT, m_Miplevels);
		m_Sampler = VkUtils::Allocator::CreateSampler((VkFilter)m_SamplerSpecification.MagFilter, (VkFilter)m_SamplerSpecification.MinFilter, (VkSamplerAddressMode)m_SamplerSpecification.Address, (VkSamplerMipmapMode)m_SamplerSpecification.Mipmaps, m_Miplevels);

		SetData((void*)pixels, imageSize);
		stbi_image_free((void*)pixels);
	}

	void VulkanImage::GenerateMipmaps(VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(context.GetPhysicalDevice()->GetVkPhysicalDevice(), imageFormat, &formatProperties);
        HZ_VERIFY(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "Texture image format does not support linear blitting!");

		VulkanCommand command = VulkanCommand(true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(command.GetVkCommandBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		command.EndAndSubmit();
	}

    void VulkanImage::Destroy()
    {
        Renderer::Free([sampler = m_Sampler, imageView = m_ImageView, image = m_Image, allocation = m_Allocation]()
        {
            const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

            if (sampler)
                vkDestroySampler(context.GetDevice()->GetVkDevice(), sampler, nullptr);
            if (imageView)
                vkDestroyImageView(context.GetDevice()->GetVkDevice(), imageView, nullptr);

            if (image != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
                VkUtils::Allocator::DestroyImage(image, allocation);
        });
    }

   static VkImageAspectFlags GetVulkanImageAspectFromImageUsage(ImageUsageFlags usage)
	{
		VkImageAspectFlags flags = 0;

		if (usage & ImageUsageFlags::Colour)
			flags = flags | VK_IMAGE_ASPECT_COLOR_BIT;
		if (usage & ImageUsageFlags::DepthStencil)
			flags = flags | VK_IMAGE_ASPECT_DEPTH_BIT;

		return flags;
	}

}
