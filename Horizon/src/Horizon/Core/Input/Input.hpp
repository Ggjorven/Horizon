#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Input/KeyCodes.hpp"
#include "Horizon/Core/Input/MouseCodes.hpp"
#include "Horizon/Core/Input/TouchCodes.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Hz
{

    // Note: The input class is implemented based on platform
	// Note 2: There are no direct way to check inputs on mobile.
	class Input
	{
	public:
	#if defined (HZ_DESKTOP_ENVIRONMENT)
		static bool IsKeyPressed(Key keycode);
		static bool IsMousePressed(MouseButton button);

		static glm::vec2 GetCursorPosition();
		static void SetCursorPosition(glm::vec2 position);

		static void SetCursorMode(CursorMode mode);
	#endif
	};

}
