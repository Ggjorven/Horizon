#include "MainLayer.hpp"

#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Input/Input.hpp>
#include <Horizon/Utils/Profiler.hpp>

#include <Horizon/RenderSpace/2D/Renderer2D.hpp>
#include <Horizon/RenderSpace/2D/BatchRenderer2D.hpp> // TODO: Remove

#include <Pulse/Utils/Random.hpp>

#include "../Extensions/ImGui/imgui/imgui.h"

static uint32_t PCG_Hash(uint32_t seed)
{
	uint32_t state = seed * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

static float RandomFloat(uint32_t& seed)
{
	seed = PCG_Hash(seed);
	return (float)seed / (float)Pulse::Numeric::Max<uint32_t>();
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

	static uint32_t seed = 0;
	for (uint32_t i = 0; i < 9000; i++)
	{
		BatchRenderer2D::AddQuad({ RandomFloat(seed) * 2.0f - 1.0f, RandomFloat(seed) * 2.0f - 1.0f, 0.0f }, { 0.05f, 0.05f }, { RandomFloat(seed), RandomFloat(seed), RandomFloat(seed), 1.0f });
	}

	Renderer2D::EndBatch();
}

void MainLayer::OnUIRender()
{
	ImGui::Begin("UI Window");

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
