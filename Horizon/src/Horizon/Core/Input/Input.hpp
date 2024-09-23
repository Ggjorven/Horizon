#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Input/KeyCodes.hpp"
#include "Horizon/Core/Input/MouseCodes.hpp"
#include "Horizon/Core/Input/JoyStickCodes.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <functional>
#include <filesystem>

namespace Hz
{

	using DragDropCallbackFn = std::function<void(const std::vector<std::filesystem::path>&)>;

    // Note: The input class is implemented based on platform
	// Note 2: There are no direct way to check inputs on mobile.
	class Input
	{
	public:
		// Keyboard & Mouse
		static bool IsKeyPressed(Key keycode);
		static bool IsMousePressed(MouseButton button);

		static glm::vec2 GetCursorPosition();
		static void SetCursorPosition(glm::vec2 position);

		static void SetCursorMode(CursorMode mode);

		static std::string GetClipboard();
		static void SetClipboard(const std::string& input);

		// Drag & Drop
		static void SetDropCallback(DragDropCallbackFn&& callback);

		// Gamepad & Joystick
		static std::string GetJoyStickName(JoyStick joyStick);
		static std::string GetGamepadName(JoyStick joyStick);

		static bool JoyStickPresent(JoyStick joyStick);
		static bool JoyStickIsGamepad(JoyStick joyStick);

		static std::vector<float> GetJoyStickAxes(JoyStick joyStick);
		static std::vector<JoyStickState> GetJoyStickButtons(JoyStick joyStick);
		static std::vector<JoyStickHatState> GetJoyStickHats(JoyStick joyStick);

		static GamepadState GetGamepadState(JoyStick joyStick);
	};

}
