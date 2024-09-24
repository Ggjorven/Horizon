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

		// Note: OnUIRender doesn't get called by default, since there's no default UI library
		// if you wish to use this function add your own custom extension which sets UINeeded to true.
		virtual void OnUIRender() {}
	};

}