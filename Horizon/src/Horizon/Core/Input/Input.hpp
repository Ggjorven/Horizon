#pragma once

#include <Horizon/Core/Input/KeyCodes.hpp>
#include <Horizon/Core/Input/MouseCodes.hpp>

#include <glm/glm.hpp>

namespace Hz
{

    // Note: The input class is implemented based on platform
	class Input
	{
	public:
		bool IsKeyPressed(Key keycode);
		bool IsMousePressed(MouseButton button);

		glm::vec2 GetCursorPosition();
		void SetCursorPosition(glm::vec2 position);

		void SetCursorMode(CursorMode mode);
	};

}