#include "hzpch.h"
#include "Image.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanImage.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

#include <Pulse/Enum/Enum.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Hz::Enum::Bitwise;

namespace Hz
{

	///////////////////////////////////////////////////////////
	// Specifications
	///////////////////////////////////////////////////////////
	ImageSpecification::ImageSpecification(uint32_t width, uint32_t height, ImageUsageFlags flags)
		: Usage(ImageUsage::Size), Width(width), Height(height), Flags(flags)
	{
	}

	ImageSpecification::ImageSpecification(const std::filesystem::path& path, ImageUsageFlags flags)
		: Usage(ImageUsage::File), Path(path), Flags(flags)
	{
	}

	///////////////////////////////////////////////////////////
	// Core class
	///////////////////////////////////////////////////////////
    Ref<Image> Image::Create(const ImageSpecification& specs, const SamplerSpecification& samplerSpecs)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanImage>::Create(specs, samplerSpecs);

        return nullptr;
    }

}
