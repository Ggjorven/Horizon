#include "CustomApp.hpp"

#include <Horizon/Core/Logging.hpp>

#include <vector>

// Position (vec2) and Color (vec3)
const std::vector<float> vertexData = {
    // Positions        // Colors
    0.0f,  0.5f,        1.0f, 0.0f, 0.0f, // Vertex 1: Top vertex, Red
    -0.5f, -0.5f,       0.0f, 1.0f, 0.0f, // Vertex 2: Bottom-left vertex, Green
    0.5f, -0.5f,        0.0f, 0.0f, 1.0f  // Vertex 3: Bottom-right vertex, Blue
};

CustomApp::CustomApp()
{
    WindowSpecification windowSpecs = { 1280, 720, "Window", [this](Event& e){ EventCallback(e); }};
    RendererSpecification rendererSpecs = { BufferCount::Triple, false };
    m_Window = Window::Create(windowSpecs, rendererSpecs);

    m_Renderpass = Renderpass::Create({
        .ColourAttachment = GraphicsContext::GetSwapChainImages(),
        .ColourClearColour = { 0.0f, 0.0f, 0.0, 1.0f },
        .PreviousColourImageLayout = ImageLayout::PresentSrcKHR,
        .FinalColourImageLayout = ImageLayout::PresentSrcKHR
    });

    m_DescriptorSets = DescriptorSets::Create({ });

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

    m_VertexBuffer = VertexBuffer::Create({ .Usage = BufferMemoryUsage::GPU }, (void*)vertexData.data(), sizeof(vertexData[0]) * vertexData.size());

    m_Pipeline = Pipeline::Create({
        .Bufferlayout = BufferLayout({
            { DataType::Float2, 0, "inPosition" },
            { DataType::Float3, 1, "inColor" },
        }),
        .Cullingmode = CullingMode::None,
    }, m_DescriptorSets, shader, m_Renderpass);
}

CustomApp::~CustomApp()
{
    m_Pipeline.Reset();
    m_Renderpass.Reset();
    m_DescriptorSets.Reset();
    m_VertexBuffer.Reset();
}

void CustomApp::Run()
{
    Pulse::Time::Timer deltaTimer = {};
    Pulse::Time::Timer intervalTimer = {};
    double interval = 0.300; // Seconds
    uint32_t FPS = 0;

    while (m_Running)
    {
        deltaTimer.Reset();

        Renderer::BeginFrame();

        Renderer::Begin(m_Renderpass);

        m_Pipeline->Use(m_Renderpass->GetCommandBuffer());
        m_VertexBuffer->Bind(m_Renderpass->GetCommandBuffer());

        Renderer::Draw(m_Renderpass->GetCommandBuffer(), 3);

        Renderer::End(m_Renderpass);
        Renderer::Submit(m_Renderpass);

        Renderer::EndFrame();
        m_Window->SwapBuffers();
        m_Window->PollEvents();

        // Update FPS
        FPS++;
        if (intervalTimer.ElapsedSeconds() >= interval)
        {
            HZ_LOG_TRACE("Deltatime: {0:.4f}ms | FPS: {1} ({2} / {3:.5f})", deltaTimer.ElapsedMillis(), static_cast<uint32_t>(FPS / intervalTimer.ElapsedSeconds()), FPS, intervalTimer.ElapsedSeconds());

            intervalTimer.Reset();
            FPS = 0;
        }
    }
}

void CustomApp::EventCallback(Event& e)
{
    EventHandler handler(e);

    handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
    {
        Renderer::Recreate(e.GetWidth(), e.GetHeight(), m_Window->IsVSync());
        return false;
    });
    handler.Handle<WindowCloseEvent>([this](WindowCloseEvent& e) -> bool
    {
        m_Running = false;
        return false;
    });
}
