#include "hzpch.h"
#include "Image.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanImage.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

#include <Pulse/Enum/Enum.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
	Image::Image(const ImageSpecification& specs, const SamplerSpecification& samplerSpecs)
        : m_Instance(new Image::ImageType(specs, samplerSpecs))
	{
    }

    Image::Image(ImageType* src)
        : m_Instance(src)
    {
    }

    Image::~Image()
	{
        delete m_Instance;
	}

    void Image::SetData(void* data, size_t size)
	{
        m_Instance->SetData(data, size);
	}

	void Image::Resize(uint32_t width, uint32_t height)
	{
        m_Instance->Resize(width, height);
	}

    void Image::Transition(ImageLayout initial, ImageLayout final)
    {
        m_Instance->Transition(initial, final);
	}

    const ImageSpecification& Image::GetSpecification() const
    {
        return m_Instance->GetSpecification();
    }

    Ref<Image> Image::Create(const ImageSpecification &specs, const SamplerSpecification& samplerSpecs)
    {
        return Ref<Image>::Create(specs, samplerSpecs);
    }

}
