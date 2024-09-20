#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

namespace Hz
{

	// Note: This class is used for easily adding extended
	// functionality to the Horizon library.
	// Note 2: This class is currently unused.
	class Extension
	{
	public:
		Extension() = default;
		virtual ~Extension() = default;

		virtual void OnInitBegin() {}
		virtual void OnInitEnd() {}

		virtual void OnDestroyBegin() {}
		virtual void OnDestroyEnd() {}

		virtual void OnUpdateBegin(float deltaTime) {}
		virtual void OnUpdateEnd(float deltaTime) {}

		virtual void OnRenderBegin() {}
		virtual void OnRenderEnd() {}

		virtual void OnUIBegin() {}
		virtual void OnUIEnd() {}

		virtual void OnEvent(Hz::Event& e) {}
	};

}