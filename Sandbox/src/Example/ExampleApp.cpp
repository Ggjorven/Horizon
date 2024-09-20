#include "ExampleApp.hpp"

#include <Horizon/Core/Logging.hpp>

#include <Horizon/Math/Transforms.hpp>

#include "../Extensions/ImGui/imgui/imgui.h"

ExampleApp::ExampleApp()
{
    WindowSpecification windowSpecs = { 1280, 720, "Window", [this](Event& e){ EventCallback(e); }};
    RendererSpecification rendererSpecs = { BufferCount::Triple, false };
    m_Window = Window::Create(windowSpecs, rendererSpecs);

    m_ImGui.OnInitEnd();

    m_Renderpass = Renderpass::Create({
        .ColourAttachment = GraphicsContext::GetSwapChainImages(),
        .ColourClearColour = { 0.0f, 0.0f, 0.0, 1.0f },
        .PreviousColourImageLayout = ImageLayout::PresentSrcKHR,
        .FinalColourImageLayout = ImageLayout::PresentSrcKHR,

        .DepthAttachment = GraphicsContext::GetDepthImage(),
        .PreviousDepthImageLayout = ImageLayout::DepthStencil,
        .FinalDepthImageLayout = ImageLayout::DepthStencil,
    });

    m_DescriptorSets = DescriptorSets::Create({
        DescriptorSetGroup(1, DescriptorSetLayout(0, {
            { DescriptorType::UniformBuffer, 0, "u_Camera", ShaderStage::Vertex },
            { DescriptorType::CombinedImageSampler, 1, "u_Albedo", ShaderStage::Fragment }
        }))
    });

    Ref<Shader> shader = Shader::Create({
        .ShaderCode = {
            { ShaderStage::Vertex, ShaderCompiler::Compile(ShaderStage::Vertex, Shader::ReadGLSL("Sandbox/Example/shader.vert.glsl")) },
            { ShaderStage::Fragment, ShaderCompiler::Compile(ShaderStage::Fragment, Shader::ReadGLSL("Sandbox/Example/shader.frag.glsl")) }
        }
    });

    PipelineSpecification pipelineSpecs = {
        .Bufferlayout = StaticMeshVertex::GetLayout(),
        .Cullingmode = CullingMode::None,
    };
    pipelineSpecs.PushConstants[ShaderStage::Vertex] = { .Offset = 0, .Size = sizeof(glm::mat4) }; // Model matrix
    m_Pipeline = Pipeline::Create(pipelineSpecs, m_DescriptorSets, shader, m_Renderpass);

    m_CameraBuffer = UniformBuffer::Create({ .Usage = BufferMemoryUsage::CPUToGPU }, sizeof(glm::mat4) * 2);

    m_Mesh = StaticMesh::Create("Sandbox/Example/viking_room.obj");

    m_Texture = Image::Create({ "Sandbox/Example/viking_room.png" });

    auto& set = m_DescriptorSets->GetSets(0)[0];
    set->UploadAll({ Uploadable(m_Texture, m_DescriptorSets->GetLayout(0).GetDescriptorByName("u_Albedo")) });
}

ExampleApp::~ExampleApp()
{
    m_ImGui.OnDestroyBegin();

    m_Pipeline.Reset();
    m_Renderpass.Reset();
    m_DescriptorSets.Reset();

    m_Mesh.Reset();
    m_Texture.Reset();
}

void ExampleApp::Run()
{
    Pulse::Time::Timer deltaTimer = {};
    Pulse::Time::Timer intervalTimer = {};
    double interval = 0.300; // Seconds
    uint32_t FPS = 0;

    // The model is static
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    //Math::ComposeTransform(modelMatrix, { 2.0f, -1.0f, 0.0f }, { -90.0f, -90.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
    //Math::ComposeTransform(modelMatrix, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });

    while (m_Running)
    {
        float deltaTime = (float)deltaTimer.ElapsedSeconds();
        deltaTimer.Reset();

        m_Camera.OnUpdate(deltaTime);

        struct CameraData
        {
            glm::mat4 View;
            glm::mat4 Proj;
        };
        CameraData data = { m_Camera.GetViewMatrix(), m_Camera.GetProjectionMatrix() };

        m_CameraBuffer->SetData((void*)&data, sizeof(glm::mat4) * 2);

        Renderer::BeginFrame();

        Renderer::Begin(m_Renderpass);

        auto& set = m_DescriptorSets->GetSets(0)[0];
        set->Upload({ Uploadable(m_CameraBuffer, m_DescriptorSets->GetLayout(0).GetDescriptorByName("u_Camera")) });

        m_Pipeline->Use(m_Renderpass->GetCommandBuffer());
        m_Mesh->GetVertexBuffer()->Bind(m_Renderpass->GetCommandBuffer());
        m_Mesh->GetIndexBuffer()->Bind(m_Renderpass->GetCommandBuffer());

        m_Pipeline->PushConstant(m_Renderpass->GetCommandBuffer(), ShaderStage::Vertex, (void*)&modelMatrix);

        set->Bind(m_Pipeline, m_Renderpass->GetCommandBuffer());

        Renderer::DrawIndexed(m_Renderpass->GetCommandBuffer(), m_Mesh->GetIndexBuffer());

        Renderer::End(m_Renderpass);
        Renderer::Submit(m_Renderpass);



        m_ImGui.OnUIBegin();
        
        ImGui::Begin("A");
        ImGui::End();

        m_ImGui.OnUIEnd();



        Renderer::EndFrame();
        m_Window->SwapBuffers();
        m_Window->PollEvents();

        // Update FPS
        FPS++;
        if (intervalTimer.ElapsedSeconds() >= interval)
        {
            HZ_LOG_TRACE("Deltatime: {0:.4f}ms | FPS: {1} ({2} / {3:.5f})", deltaTime * 1000.0f, static_cast<uint32_t>(FPS / intervalTimer.ElapsedSeconds()), FPS, intervalTimer.ElapsedSeconds());

            intervalTimer.Reset();
            FPS = 0;
        }
    }
}

void ExampleApp::EventCallback(Event& e)
{
    EventHandler handler(e);

    handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
    {
        Renderer::Recreate(e.GetWidth(), e.GetHeight(), m_Window->IsVSync());
        m_Renderpass->Resize(e.GetWidth(), e.GetHeight());
        return false;
    });
    handler.Handle<WindowCloseEvent>([this](WindowCloseEvent& e) -> bool
    {
        m_Running = false;
        return false;
    });

    m_ImGui.OnEvent(e);
}
