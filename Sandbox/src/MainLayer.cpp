#include "MainLayer.hpp"

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Input/Input.hpp>
#include <Horizon/Utils/Profiler.hpp>

#include <Horizon/RenderSpace/2D/Renderer2D.hpp>
#include <Horizon/RenderSpace/2D/BatchRenderer2D.hpp> // TODO: Remove

#include "Extensions/ImGui/imgui/imgui.h"

static void RemoveControllerDrift(float& x, float& y)
{
	constexpr static float DeadZone = 0.1f;

	// Calculate the magnitude
	float magnitude = std::sqrt(x * x + y * y);

	// Check if within the dead zone
	if (magnitude < DeadZone)
	{
		x = 0.0f;
		y = 0.0f;
		return;
	}
}

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

	BatchRenderer2D::AddQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });

	Renderer2D::EndBatch();
}

void MainLayer::OnUIRender()
{
	ImGui::Begin("Test UI Window");

	ImGui::Text("Text");
	ImGui::Button("Button");

	if (Input::JoyStickPresent(JoyStick::J1))
	{
		auto axes = Input::GetJoyStickAxes(JoyStick::J1);
		RemoveControllerDrift(axes[0], axes[1]);
	
		ImGui::Text(Text::Format("X: {}", axes[0]).c_str());
		ImGui::Text(Text::Format("Y: {}", axes[1]).c_str());
	}
	
	ImGui::End();
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