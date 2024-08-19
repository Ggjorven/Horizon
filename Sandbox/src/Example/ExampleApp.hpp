#pragma once

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Window.hpp>

#include <Horizon/Renderer/Renderer.hpp>
#include <Horizon/Renderer/GraphicsContext.hpp>

#include <Horizon/Renderer/Shader.hpp>
#include <Horizon/Renderer/Buffers.hpp>
#include <Horizon/Renderer/Pipeline.hpp>
#include <Horizon/Renderer/Renderpass.hpp>
#include <Horizon/Renderer/Descriptors.hpp>
#include <Horizon/Renderer/Image.hpp>

#include <Horizon/Utils/StaticMesh.hpp>

#include "Example/Camera.hpp"

#include <Pulse/Time/Timer.hpp>

using namespace Hz;

class ExampleApp
{
public:
    ExampleApp();
    ~ExampleApp();

    void Run();

private:
    void EventCallback(Event& e);

private:
    Application m_Application;          // Note: The Application
    Ref<Window> m_Window = nullptr;     // and Window need to stay alive the longest.
    bool m_Running = true;

    Ref<Pipeline> m_Pipeline = nullptr;
    Ref<Renderpass> m_Renderpass = nullptr;
    Ref<DescriptorSets> m_DescriptorSets = nullptr;

    Ref<UniformBuffer> m_CameraBuffer = nullptr;

    Camera m_Camera = {};

    Ref<StaticMesh> m_Mesh = nullptr;
    Ref<Image> m_Texture = nullptr;
};
