#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Pipeline.hpp"
#include "Horizon/Renderer/Buffers.hpp"
#include "Horizon/Renderer/Image.hpp"
// Note: I purposefully don't forward declare ^ since I want
// the user to be able to just include the Renderer (this).

#include <glm/glm.hpp>

#include <functional>

namespace Hz
{

    ///////////////////////////////////////////////////////////
    // Specifications
    ///////////////////////////////////////////////////////////
    enum class ExecutionPolicy : uint8_t
    {
        InOrder = 1 << 0,           // Execute commands sequentially, submits to waited on by next (WaitForPrevious) commandBuffer
        Parallel = 1 << 1,          // Execute commands in parallel but synchronized by the frame
        WaitForPrevious = 1 << 2,   // Wait for the completion of the previous (InOrder) command buffer
        NoWait = 1 << 3             // Do not wait for the previous (InOrder) command buffer
    };
    ENABLE_BITWISE(ExecutionPolicy)

    enum class Queue    : uint8_t  { Graphics, Present, Compute };

    struct DynamicRenderState
    {
    public:
        Ref<Image> ColourAttachment = nullptr; // If using swapchain images use Renderer::GetAcquiredImage() as the index.
        LoadOperation ColourLoadOp = LoadOperation::Clear;
        StoreOperation ColourStoreOp = StoreOperation::Store;
        glm::vec4 ColourClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

        Ref<Image> DepthAttachment = nullptr;
        LoadOperation DepthLoadOp = LoadOperation::Clear;
        StoreOperation DepthStoreOp = StoreOperation::Store;
        float DepthClearValue = 1.0f;
    };

    using FreeFunction = std::function<void()>;

    ///////////////////////////////////////////////////////////
    // Core class
    ///////////////////////////////////////////////////////////
    class Renderer
    {
    public:
        static void Init(const RendererSpecification& specs);
        static bool Initialized();
        static void Destroy();

        // Has to be manually called by user on window resize events
        static void Recreate(uint32_t width, uint32_t height, const bool vsync);

        static void BeginFrame();
        static void EndFrame();
        static void Present();

        // Note: These are only when you actually want to use dynamic rendering, 'static' rendering using renderpasses can be done just with Begin() etc.
        // Note 2: You still have to call Begin, End & Submit for the CommandBuffer yourself.
        static void BeginDynamic(Ref<CommandBuffer> cmdBuf, DynamicRenderState&& state);
        static void EndDynamic(Ref<CommandBuffer> cmdBuf);

        // Note: Used for dynamic rendering
        static void SetViewportAndScissor(Ref<CommandBuffer> cmdBuf, uint32_t width, uint32_t height);

        // Execution of Renderpasses/CommandBuffers
        static void Begin(Ref<CommandBuffer> cmdBuf);
        static void Begin(Ref<Renderpass> renderpass);
        static void End(Ref<CommandBuffer> cmdBuf);
        static void End(Ref<Renderpass> renderpass);
        static void Submit(Ref<CommandBuffer> cmdBuf, ExecutionPolicy policy = ExecutionPolicy::InOrder | ExecutionPolicy::WaitForPrevious, Queue queue = Queue::Graphics, PipelineStage waitStage = PipelineStage::ColourAttachmentOutput, const std::vector<Ref<CommandBuffer>>& waitOn = {});
        static void Submit(Ref<Renderpass> renderpass, ExecutionPolicy policy = ExecutionPolicy::InOrder | ExecutionPolicy::WaitForPrevious, Queue queue = Queue::Graphics, PipelineStage waitStage = PipelineStage::ColourAttachmentOutput, const std::vector<Ref<CommandBuffer>>& waitOn = {});

        static void Draw(Ref<CommandBuffer> cmdBuf, uint32_t vertexCount = 3, uint32_t instanceCount = 1);
        static void DrawIndexed(Ref<CommandBuffer> cmdBuf, uint32_t indexCount, uint32_t instanceCount = 1);
        static void DrawIndexed(Ref<CommandBuffer> cmdBuf, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount = 1);

        static void Free(FreeFunction&& func); // Adds to the renderfree queue
        static void FreeObjects(); // Executes the free queue

        static uint32_t GetAcquiredImage();
        static uint32_t GetCurrentFrame();
        static const RendererSpecification& GetSpecification();
    };

}
