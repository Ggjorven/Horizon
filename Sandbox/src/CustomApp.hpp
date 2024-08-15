#pragma once

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Window.hpp>

#include <Horizon/Renderer/Renderer.hpp>
#include <Horizon/Renderer/GraphicsContext.hpp>

#include <Horizon/Renderer/Shader.hpp>
#include <Horizon/Renderer/Pipeline.hpp>
#include <Horizon/Renderer/Renderpass.hpp>
#include <Horizon/Renderer/Descriptors.hpp>

#include <Pulse/Time/Timer.hpp>

using namespace Hz;

class CustomApp
{
public:
    CustomApp();
    ~CustomApp();

    void Run();

private:
    void EventCallback(Event& e);

private:
    Application m_Application;
    Ref<Window> m_Window = nullptr; // Note: Windows needs to stay alive the longest
    bool m_Running = true;

    Ref<Pipeline> m_Pipeline = nullptr;
    Ref<Renderpass> m_Renderpass = nullptr;
    Ref<DescriptorSets> m_DescriptorSets = nullptr;
    Ref<VertexBuffer> m_VertexBuffer = nullptr;
};
