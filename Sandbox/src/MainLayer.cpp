#include "MainLayer.hpp"

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Input/Input.hpp>
#include <Horizon/Utils/Profiler.hpp>

#include <Horizon/RenderSpace/2D/Renderer2D.hpp>
#include <Horizon/RenderSpace/2D/BatchRenderer2D.hpp> // TODO: Remove

#include "../Extensions/ImGui/imgui/imgui.h"

void MainLayer::OnInit()
{
	Renderer2D::Init();
}

void MainLayer::OnDestroy()
{
	Renderer2D::Destroy();
}

void MainLayer::OnUpdate(float deltaTime)
{
	constexpr float s_Threshold = 0.3f;
	static float s_Timer = 0.0f;
	static uint32_t s_FPS = 0u;

	s_Timer += deltaTime;
	++s_FPS;

	if (s_Timer >= s_Threshold)
	{
		Window::Get().SetTitle(Text::Format("Sandbox | Horizon Application | FPS: {0}", static_cast<uint32_t>((static_cast<float>(s_FPS) / s_Timer))));

		s_FPS = 0u;
		s_Timer = 0.0f;
	}
}

void MainLayer::OnRender()
{
	Renderer2D::BeginBatch();

	BatchRenderer2D::AddQuad({ -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });

	Renderer2D::EndBatch();
}

void MainLayer::OnUIRender()
{
}

void MainLayer::OnEvent(Event& e)
{
	EventHandler handler(e);

	handler.Handle<JoyStickConnectedEvent>([](JoyStickConnectedEvent& e) -> bool
	{
		HZ_LOG_TRACE("Controller connected");
		return false;
	});
	handler.Handle<JoyStickDisconnectedEvent>([](JoyStickDisconnectedEvent& e) -> bool
	{
		HZ_LOG_TRACE("Controller disconnected");
		return false;
	});
}
