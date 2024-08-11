#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Logging.hpp>
#include <Horizon/Core/Window.hpp>

#include <Horizon/Renderer/Renderer.hpp>
#include <Horizon/Renderer/GraphicsContext.hpp>

#include <Horizon/Renderer/Shader.hpp>
#include <Horizon/Renderer/Pipeline.hpp>
#include <Horizon/Renderer/Renderpass.hpp>
#include <Horizon/Renderer/Descriptors.hpp>

#include <Pulse/Time/Timer.hpp>

using namespace Hz;

static bool s_ShouldClose = false;
static void EventCallback(Event& e)
{
    EventHandler handler(e);
    handler.Handle<WindowResizeEvent>([](WindowResizeEvent& e) -> bool
    {
        Renderer::Recreate(e.GetWidth(), e.GetHeight(), Renderer::GetSpecification().VSync);
        return false;
    });
    handler.Handle<WindowCloseEvent>([](WindowCloseEvent& e) -> bool
    {
        s_ShouldClose = true;
        return false;
    });
}

int main(int argc, char* argv[])
{
    Application application = {};

    WindowSpecification windowSpecs = { 1280, 720, "Window", &EventCallback };
    RendererSpecification rendererSpecs = { BufferCount::Triple, false };
    Ref<Window> window = Window::Create(windowSpecs, rendererSpecs);

    Ref<Renderpass> renderpass = Renderpass::Create({
        .ColourAttachment = GraphicsContext::GetSwapChainImages(),
        .ColourClearColour = { 0.0f, 0.0f, 0.0, 1.0f },
        .PreviousColourImageLayout = ImageLayout::PresentSrcKHR,
        .FinalColourImageLayout = ImageLayout::PresentSrcKHR
    });

    Ref<DescriptorSets> descriptorSets = DescriptorSets::Create({ });

    Ref<Shader> shader = Shader::Create({
        .ShaderCode = {
            { ShaderStage::Vertex, ShaderCompiler::Compile<ShadingLanguage::GLSL>(ShaderStage::Vertex, R"(
#version 460

layout(location = 0) in vec2 inPosition; // Input vertex position
layout(location = 1) in vec3 inColor;    // Input vertex color

layout(location = 0) out vec3 fragColor; // Output color to fragment shader

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0); // Transform vertex position
    fragColor = inColor;                      // Pass color to fragment shader
}
            )") },
            { ShaderStage::Fragment, ShaderCompiler::Compile<ShadingLanguage::GLSL>(ShaderStage::Fragment, R"(
#version 460

layout(location = 0) in vec3 fragColor; // Input color from vertex shader

layout(location = 0) out vec4 outColor; // Output color to framebuffer

void main() {
    outColor = vec4(fragColor, 1.0); // Set the output color (with full opacity)
}
            )") },
        }
    });

    // Position (vec2) and Color (vec3)
    const std::vector<float> vertexData = {
        // Positions        // Colors
        0.0f,  0.5f,        1.0f, 0.0f, 0.0f, // Vertex 1: Top vertex, Red
        -0.5f, -0.5f,       0.0f, 1.0f, 0.0f, // Vertex 2: Bottom-left vertex, Green
        0.5f, -0.5f,        0.0f, 0.0f, 1.0f  // Vertex 3: Bottom-right vertex, Blue
    };

    Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create({ .Usage = BufferMemoryUsage::GPU }, (void*)vertexData.data(), sizeof(vertexData[0]) * vertexData.size());

    // BufferElement(DataType type, uint32_t location, const std::string& name);
    Ref<Pipeline> pipeline = Pipeline::Create({
        .Bufferlayout = BufferLayout({
            { DataType::Float2, 0, "inPosition" },
            { DataType::Float3, 1, "inColor" },
        }),
        .Cullingmode = CullingMode::None,
    }, descriptorSets, shader, renderpass);

    Pulse::Time::Timer deltaTimer = {};
    Pulse::Time::Timer intervalTimer = {};
    double interval = 0.300; // Seconds
    uint32_t FPS = 0;

    while (!s_ShouldClose)
    {
        deltaTimer.Reset();

        window->PollEvents();
        Renderer::BeginFrame();

        Renderer::Begin(renderpass);

        pipeline->Use(renderpass->GetCommandBuffer());
        vertexBuffer->Bind(renderpass->GetCommandBuffer());

        Renderer::Draw(renderpass->GetCommandBuffer(), 3);

        Renderer::End(renderpass);
        Renderer::Submit(renderpass);

        Renderer::EndFrame();
        window->SwapBuffers();

        // Update FPS
        FPS++;
        if (intervalTimer.ElapsedSeconds() >= interval)
        {
            HZ_LOG_TRACE("Deltatime: {0:.4f}ms | FPS: {1} ({2} / {3:.5f})", deltaTimer.ElapsedMillis(), static_cast<uint32_t>(FPS / intervalTimer.ElapsedSeconds()), FPS, intervalTimer.ElapsedSeconds());

            intervalTimer.Reset();
            FPS = 0;
        }
    }

    renderpass.Reset();
    return 0;
}
