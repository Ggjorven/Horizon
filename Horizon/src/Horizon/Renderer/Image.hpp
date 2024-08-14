#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <cstdint>
#include <variant>
#include <filesystem>
#include <type_traits>

namespace Hz
{

    ///////////////////////////////////////////////////////////
	// Specifications
	///////////////////////////////////////////////////////////
	enum class ImageUsage : uint8_t		    { None = 0, Size, File };
	enum class ImageUsageFlags : uint32_t
	{
        None = 0,
        TransferSrc = 0x00000001,   // May be added by default if image is loaded by another image for example
        TransferDst = 0x00000002,   // May be added by default if image is loaded by another image for example
        Sampled = 0x00000004,
        Storage = 0x00000008,
        Colour = 0x00000010,
        DepthStencil = 0x00000020,
        Transient = 0x00000040,
        Input = 0x00000080,
        VideoDecodeDstKHR = 0x00000400,
        VideoDecodeSrcKHR = 0x00000800,
        VideoDecodeDpbKHR = 0x00001000,
        FragmentDensityMapEXT = 0x00000200,
        FragmentShadingRateKHR = 0x00000100,
        HostTransferEXT = 0x00400000,
        VideoEncodeDstKHR = 0x00002000,
        VideoEncodeSrcKHR = 0x00004000,
        VideoEncodeDpbKHR = 0x00008000,
        FeedbackLoopEXT = 0x00080000,
        InvocationMaskHuawei = 0x00040000,
        SampleWeightQCOM = 0x00100000,
        SampleBlockMatchQCOM = 0x00200000,
        ShadingRateImageNV = FragmentShadingRateKHR,
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

        bool MipMaps = true;

	public:
		ImageSpecification() = default;
		ImageSpecification(uint32_t width, uint32_t height, ImageUsageFlags flags);
		ImageSpecification(const std::filesystem::path& path, ImageUsageFlags flags);
		~ImageSpecification() = default;
	};

    enum class FilterMode
    {
        Nearest = 0,
        Linear,
        CubicEXT = 1000015000,
        CubicIMG = CubicEXT,
    };

    enum class AddressMode
    {
        Repeat = 0,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge,
        MirrorClampToEdgeKHR = MirrorClampToEdge,
    };

    enum class MipmapMode
    {
        Nearest = 0,
        Linear
    };

    struct SamplerSpecification
    {
    public:
        FilterMode MagFilter = FilterMode::Linear;
        FilterMode MinFilter = FilterMode::Linear;
        AddressMode Address = AddressMode::Repeat; // For U, V & W
        MipmapMode Mipmaps = MipmapMode::Linear;

    public:
        SamplerSpecification() = default;
        ~SamplerSpecification() = default;
    };

	///////////////////////////////////////////////////////////
    // Core class
	///////////////////////////////////////////////////////////
    class Image : public RefCounted
    {
    public:
        Image() = default;
        virtual ~Image() = default;

        virtual void SetData(void* data, size_t size) = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual void Transition(ImageLayout initial, ImageLayout final) = 0;

        virtual const ImageSpecification& GetSpecification() const = 0;

        static Ref<Image> Create(const ImageSpecification& specs, const SamplerSpecification& samplerSpecs = {});
    };

}
