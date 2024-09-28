#pragma once

#include <Horizon/Core/Core.hpp>
#include <Horizon/Core/Layer.hpp>

#include <Horizon/Renderer/Image.hpp>

#include "Core/Camera.hpp"

using namespace Hz;

class MainLayer : public Layer
{
public:
	void OnInit() override;
	void OnDestroy() override;

	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnUIRender() override;

	void OnEvent(Event& e) override;

private:
	Ref<Image> m_Texture0 = nullptr;
	Ref<Image> m_Texture1 = nullptr;

	Ref<Camera> m_Camera = nullptr;
};