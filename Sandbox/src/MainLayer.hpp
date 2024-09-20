#pragma once

#include <Horizon/Core/Layer.hpp>

using namespace Hz;

class MainLayer : public Layer
{
public:
	void OnInit() override;
	void OnDestroy() override;

	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnEvent(Event& e) override;
};