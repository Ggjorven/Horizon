#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/Image.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Hz
{

    class VulkanSwapChain;

    class VulkanImage
	{
	public:
		VulkanImage(const ImageSpecification& specs);
        VulkanImage(const ImageSpecification& specs, const VkImage image, const VkImageView imageView); // For swapchain
		~VulkanImage();

		void SetData(void* data, size_t size);

		void Resize(uint32_t width, uint32_t height);

		void Transition(ImageLayout initial, ImageLayout final);

		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

		inline uint32_t GetWidth() const { return m_Specification.Width; }
		inline uint32_t GetHeight() const { return m_Specification.Height; }

		inline const VkImage GetVkImage() const { return m_Image; }
		inline const VmaAllocation GetVmaAllocation() const { return m_Allocation; }
		inline const VkImageView GetVkImageView() const { return m_ImageView; }
		inline const VkSampler GetVkSampler() const { return m_Sampler; }

    private:
        void CreateImage(uint32_t width, uint32_t height);
		void CreateImage(const std::filesystem::path& path);

		void GenerateMipmaps(VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	private:
		ImageSpecification m_Specification = {};

		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;

		uint32_t m_Miplevels = 1;

        friend class VulkanSwapChain;
	};

}
