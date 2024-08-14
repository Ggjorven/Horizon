#pragma once

#include <cstdint>

namespace Hz
{

    enum class RenderingAPI : uint8_t       { Vulkan };
    enum class BufferCount : uint8_t		{ Single = 1, Double = 2, Triple = 3 };	// Amount of buffers

	struct RendererSpecification
	{
	public:
        // Static API selection
        inline static constexpr RenderingAPI API = RenderingAPI::Vulkan;
    public:
		BufferCount Buffers;
		bool VSync;

	public:
		constexpr RendererSpecification(BufferCount buffers = BufferCount::Triple, bool vsync = true)
			: Buffers(buffers), VSync(vsync)
		{
        }
		constexpr ~RendererSpecification() = default;
	};

}
