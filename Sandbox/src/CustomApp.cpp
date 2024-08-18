#include "CustomApp.hpp"

#include <Horizon/Core/Logging.hpp>

#include <vector>

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

    m_DescriptorSets = DescriptorSets::Create({
        DescriptorSetGroup(1, DescriptorSetLayout(0, {
            { DescriptorType::CombinedImageSampler, 1, "u_Albedo", ShaderStage::Fragment }
        }))
    });

    Ref<Shader> shader = Shader::Create({
        .ShaderCode = {
            { ShaderStage::Vertex, ShaderCompiler::Compile(ShaderStage::Vertex, Shader::ReadGLSL("Sandbox/temp/shader.vert.glsl")) },
            { ShaderStage::Fragment, ShaderCompiler::Compile(ShaderStage::Fragment, Shader::ReadGLSL("Sandbox/temp/shader.frag.glsl")) }
        }
    });

    m_Pipeline = Pipeline::Create({
        .Bufferlayout = MeshVertex::GetLayout(),
        .Cullingmode = CullingMode::None,
    }, m_DescriptorSets, shader, m_Renderpass);

    m_Mesh = Mesh::Create("Sandbox/temp/viking_room.obj");

    ImageSpecification specs = { "Sandbox/temp/viking_room.png" };
    specs.MipMaps = false;
    m_Texture = Image::Create(specs);

    //auto set = m_DescriptorSets->GetSets(0)[0];
    //set->UploadAll({ Uploadable(m_Texture, m_DescriptorSets->GetLayout(0).GetDescriptorByName("u_Albedo")) });
}

CustomApp::~CustomApp()
{
    m_Pipeline.Reset();
    m_Renderpass.Reset();
    m_DescriptorSets.Reset();

    m_Mesh.Reset();
    m_Texture.Reset();
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

        auto& set = m_DescriptorSets->GetSets(0)[0];
        set->Upload({ Uploadable(m_Texture, m_DescriptorSets->GetLayout(0).GetDescriptorByName("u_Albedo")) });

        m_Pipeline->Use(m_Renderpass->GetCommandBuffer());
        m_Mesh->GetVertexBuffer()->Bind(m_Renderpass->GetCommandBuffer());
        m_Mesh->GetIndexBuffer()->Bind(m_Renderpass->GetCommandBuffer());

        set->Bind(m_Pipeline, m_Renderpass->GetCommandBuffer());

        Renderer::DrawIndexed(m_Renderpass->GetCommandBuffer(), m_Mesh->GetIndexBuffer());

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
