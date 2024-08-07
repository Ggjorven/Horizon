#pragma once

#include "Horizon/Core/Memory.hpp"

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
	enum class LoadOperation : uint8_t  { Load, Clear };

	struct RenderpassSpecification
	{
	public:
		std::vector<Ref<Image>> ColourAttachment = { }; // This is a vector since the swapchain has multiple images.
		LoadOperation ColourLoadOp = LoadOperation::Clear;
		glm::vec4 ColourClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		ImageLayout PreviousColourImageLayout = ImageLayout::Undefined;
		ImageLayout FinalColourImageLayout = ImageLayout::PresentSrcKHR;

		Ref<Image> DepthAttachment = nullptr;
		LoadOperation DepthLoadOp = LoadOperation::Clear;
		ImageLayout PreviousDepthImageLayout = ImageLayout::Undefined;
		ImageLayout FinalDepthImageLayout = ImageLayout::Depth;
	};

    ///////////////////////////////////////////////////////////
    // Core class
    ///////////////////////////////////////////////////////////
    class Renderpass : public RefCounted
    {
    public:
        using RenderpassType = VulkanRenderpass;
        static_assert(std::is_same_v<RenderpassType, VulkanRenderpass>, "Unsupported rendererpass type selected.");
    public:
        Renderpass(const RenderpassSpecification& specs, Ref<CommandBuffer> commandBuffer = nullptr);
        Renderpass(RenderpassType* src);
        ~Renderpass();

        // The Begin, End & Submit function are in the Renderer

        void Resize(uint32_t width, uint32_t height);

        std::pair<uint32_t, uint32_t> GetSize() const;

		const RenderpassSpecification& GetSpecification();
		Ref<CommandBuffer> GetCommandBuffer();

        RenderpassType* Src() { return m_Instance; }

        static Ref<Renderpass> Create(const RenderpassSpecification& specs, Ref<CommandBuffer> commandBuffer = nullptr);

    private:
        RenderpassType* m_Instance;
    };


}
