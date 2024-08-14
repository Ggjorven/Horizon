#include "Application.hpp"

#include <Horizon/Renderer/Renderer.hpp>

#include "Forward+/ForwardPlusRenderer.hpp"

#include <Pulse/Enum/Enum.hpp>
#include <Pulse/Time/Timer.hpp>

using namespace Hz;

namespace Sandbox
{

    static Application* s_Instance = nullptr;

    Application::Application()
    {
        s_Instance = this;

        WindowSpecification windowSpecs = { 1280, 720, "Window", [this](Event& e){ OnEvent(e); }};
        RendererSpecification rendererSpecs = { BufferCount::Triple, false };

        m_Window = Window::Create(windowSpecs, rendererSpecs);

        m_Renderers[RenderMode::ForwardPlus] = Ref<ForwardPlusRenderer>::Create();
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    void Application::Run()
    {
        Pulse::Time::Timer deltaTimer = {};

        // TODO: Remove
        Ref<CommandBuffer> cmd = CommandBuffer::Create();

        while (m_Running)
        {
            // Update
            {
                float deltaTime = deltaTimer.ElapsedMillis();
                deltaTimer.Reset();

                m_Renderers[m_Mode]->OnUpdate((float)deltaTime);
            }

            // Render
            {
                using namespace Pulse::Enum::Bitwise;

                Renderer::BeginFrame();

                m_Renderers[m_Mode]->OnRender();

                Renderer::Begin(cmd);
                Renderer::End(cmd);
                Renderer::Submit(cmd, ExecutionPolicy::InOrder | ExecutionPolicy::WaitForPrevious);

                Renderer::EndFrame();
                m_Window->SwapBuffers();
            }

            // UI // TODO: ?
            m_Window->PollEvents();
        }
    }

    void Application::OnEvent(Event& e)
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

        m_Renderers[m_Mode]->OnEvent(e);
    }

    Application& Application::Get()
    {
        return *s_Instance;
    }

}
