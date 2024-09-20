#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

#include <string>
#include <vector>

namespace Hz
{

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnInit() {}
		virtual void OnDestroy() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnEvent(Event& e) {}
		virtual void OnUIRender() {}
	};

}