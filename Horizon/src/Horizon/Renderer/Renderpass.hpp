#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Image.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include <glm/glm.hpp>

#include <type_traits>

namespace Hz
{

    class VulkanRenderpass;

    ///////////////////////////////////////////////////////////
	// Specification
    ///////////////////////////////////////////////////////////
    enum class LoadOperation
    {
        Load = 0,
        Clear = 1,
        DontCare = 2,
        NoneKHR = 1000400000,
        NoneEXT = NoneKHR,
    };

    enum class StoreOperation
    {
        Store = 0,
        DontCare = 1,
        None = 1000301000,
        NoneKHR = None,
        NoneQCOM = None,
        NoneEXT = None,
    };

	struct RenderpassSpecification
	{
	public:
		std::vector<Ref<Image>> ColourAttachment = { }; // This is a vector since the swapchain has multiple images (most of the time).
		LoadOperation ColourLoadOp = LoadOperation::Clear;
        StoreOperation ColourStoreOp = StoreOperation::Store;
		glm::vec4 ColourClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		ImageLayout PreviousColourImageLayout = ImageLayout::Undefined;
		ImageLayout FinalColourImageLayout = ImageLayout::PresentSrcKHR;

		Ref<Image> DepthAttachment = nullptr;
		LoadOperation DepthLoadOp = LoadOperation::Clear;
        StoreOperation DepthStoreOp = StoreOperation::Store;
		ImageLayout PreviousDepthImageLayout = ImageLayout::Undefined;
		ImageLayout FinalDepthImageLayout = ImageLayout::Depth;
	};

    ///////////////////////////////////////////////////////////
    // Core class
    ///////////////////////////////////////////////////////////
    class Renderpass : public RefCounted
    {
    public:
        Renderpass() = default;
        virtual ~Renderpass() = default;

        // The Begin, End & Submit function are in the Renderer

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;

		virtual const RenderpassSpecification& GetSpecification() = 0;
		virtual Ref<CommandBuffer> GetCommandBuffer() = 0;

        static Ref<Renderpass> Create(const RenderpassSpecification& specs, Ref<CommandBuffer> commandBuffer = nullptr);
    };


}
