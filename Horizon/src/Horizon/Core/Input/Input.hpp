#pragma once

#include <Horizon/Core/Input/KeyCodes.hpp>
#include <Horizon/Core/Input/MouseCodes.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Hz
{

    // Note: The input class is implemented based on platform
	class Input
	{
	public:
		static bool IsKeyPressed(Key keycode);
		static bool IsMousePressed(MouseButton button);

		static glm::vec2 GetCursorPosition();
		static void SetCursorPosition(glm::vec2 position);

		static void SetCursorMode(CursorMode mode);
	};

}
