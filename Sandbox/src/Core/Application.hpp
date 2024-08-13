#pragma once

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Window.hpp>

#include "Core/RendererBackend.hpp"

#include "Scene/Scene.hpp"

namespace Sandbox
{

    enum class RenderMode
    {
        ForwardPlus
    };

    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

        inline Scene& GetScene() { return m_Scene; }
        inline Hz::Ref<Hz::Window> GetWindow() { return m_Window;}

        static Application& Get();

    private:
        void OnEvent(Hz::Event& e);

    private:
        Hz::Application m_App = {};
        Hz::Ref<Hz::Window> m_Window = nullptr;

        bool m_Running = true;

        RenderMode m_Mode = RenderMode::ForwardPlus;
        std::unordered_map<RenderMode, Hz::Ref<RendererBackend>> m_Renderers;

        Scene m_Scene = {};
    };

}
