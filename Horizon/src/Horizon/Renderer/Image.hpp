#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <Pulse/Enum/Enum.hpp>

#include <cstdint>
#include <variant>
#include <filesystem>
#include <type_traits>

namespace Hz
{

    using namespace Pulse::Enum::Bitwise;

    class VulkanImage;
    class VulkanRenderer;

    ///////////////////////////////////////////////////////////
	// Specifications
	///////////////////////////////////////////////////////////
	enum class ImageUsage : uint8_t		    { None = 0, Size, File };
	enum class ImageUsageFlags : uint8_t
	{
		None = 0, Sampled = 0 << 1, Storage = 1 << 1, Colour = 2 << 1, Depth = 3 << 1, Transient = 4 << 1, Input = 5 << 1,
		NoMipMaps = 6 << 1 // Note: Depth always has to have the NoMipMaps flag
	};

	enum class ImageLayout : uint32_t
	{
		Undefined = 0,
		General = 1,
		Colour = 2,
		DepthStencil = 3,
		DepthStencilRead = 4,
		ShaderRead = 5,
		TransferSrc = 6,
		TransferDst = 7,
		PreInitialized = 8,
		DepthReadStencil = 1000117000,
		DepthReadStencilRead = 1000117001,
		Depth = 1000241000,
		DepthRead = 1000241001,
		Stencil = 1000241002,
		StencilRead = 1000241003,
		Read = 1000314000,
		Attachment = 1000314001,
		PresentSrcKHR = 1000001002,
		VideoDecodeDstKHR = 1000024000,
		VideoDecodeSrcKHR = 1000024001,
		VideoDecodeDpbKHR = 1000024002,
		SharedPresentKHR = 1000111000,
		FragmentDensityMapEXT = 1000218000,
		FragmentShadingRateKHR = 1000164003,
		RenderingLocalReadKHR = 1000232000,
		VideoEncodeDstKHR = 1000299000,
		VideoEncodeSrcKHR = 1000299001,
		VideoEncodeDpbKHR = 1000299002,
		FeedbackLoopEXT = 1000339000,

		DepthReadStencilKHR = DepthReadStencil,
		DepthStencilReadKHR = DepthStencilRead,
		ShadingRateNV = FragmentShadingRateKHR,
		DepthKHR = Depth,
		DepthReadKHR = DepthRead,
		StencilKHR = Stencil,
		StencilReadKHR = StencilRead,
		ReadKHR = Read,
		AttachmentKHR = Attachment
	};
	enum class ImageFormat : uint8_t // Only default formats since there are hundreds of formats
	{
		Undefined = 0,
		RGBA = 37,
		BGRA = 44,
		sRGB = 43,
		Depth32SFloat = 126,
		Depth32SFloatS8 = 130,
		Depth24UnormS8 = 129
	};

	struct ImageSpecification
	{
	public:
		ImageUsage Usage = ImageUsage::None;
		ImageUsageFlags Flags = ImageUsageFlags::Sampled;
		ImageLayout Layout = ImageLayout::ShaderRead;
		ImageFormat Format = ImageFormat::RGBA;

		std::filesystem::path Path = {};

		uint32_t Width = 0;
		uint32_t Height = 0;

	public:
		ImageSpecification() = default;
		ImageSpecification(uint32_t width, uint32_t height, ImageUsageFlags flags);
		ImageSpecification(const std::filesystem::path& path, ImageUsageFlags flags = ImageUsageFlags::Sampled | ImageUsageFlags::Colour);
		~ImageSpecification() = default;
	};

	///////////////////////////////////////////////////////////
    // Core class
	///////////////////////////////////////////////////////////
    class Image : public RefCounted
    {
    public:
        using ImageType = VulkanImage;
        static_assert(std::is_same_v<ImageType, VulkanImage>, "Unsupported image type selected.");
    public:
        Image(const ImageSpecification& specs);
        Image(ImageType* src); // Takes ownership of passed in object
        ~Image();

        void SetData(void* data, size_t size);
        void Resize(uint32_t width, uint32_t height);
        void Transition(ImageLayout initial, ImageLayout final);

        const ImageSpecification& GetSpecification() const;

        // Returns underlying type pointer
        inline ImageType* Src() { return m_Instance; }

        static Ref<Image> Create(const ImageSpecification& specs);

    private:
        ImageType* m_Instance;

        friend class VulkanRenderer;
    };

}
