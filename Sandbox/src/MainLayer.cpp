#include "MainLayer.hpp"

#include <Horizon/Core/Application.hpp>
#include <Horizon/Utils/Profiler.hpp>

#include "../Extensions/ImGui/imgui/imgui.h"

void MainLayer::OnInit()
{
}

void MainLayer::OnDestroy()
{
}

void MainLayer::OnUpdate(float deltaTime)
{
}

void MainLayer::OnRender()
{
}

void MainLayer::OnUIRender()
{
	ImGui::Begin("Test UI Window");

	ImGui::Text("Text");
	ImGui::Button("Button");

	ImGui::End();
}

void MainLayer::OnEvent(Event& e)
{
}