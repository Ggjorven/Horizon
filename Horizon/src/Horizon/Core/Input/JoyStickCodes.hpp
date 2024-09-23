#pragma once

#include "Horizon/Core/Core.hpp"

#include <cstdint>
#include <array>

namespace Hz
{

	// From glfw3.h
	enum class JoyStick : uint8_t
	{
		J1 = 0,
		J2 = 1,
		J3 = 2,
		J4 = 3,
		J5 = 4,
		J6 = 5,
		J7 = 6,
		J8 = 7,
		J9 = 8,
		J10 = 9,
		J11 = 10,
		J12 = 11,
		J13 = 12,
		J14 = 13,
		J15 = 14,
		J16 = 15,

		Last = J16,
	};

	enum class JoyStickState : uint8_t
	{
		Released = 0,
		Pressed = 1,
	};

	enum class JoyStickHatState : uint8_t
	{
		Centered = 0,
		Up = 1 << 0,
		Right = 1 << 1,
		Down = 1 << 2,
		Left = 1 << 3,

		RightUp = Right | Up,
		RightDown = Right | Down,
		LeftUp = Left | Up,
		LeftDown = Left | Down,
	};
	ENABLE_BITWISE(JoyStickHatState)

	enum class GamepadButton : uint8_t
	{
		A = 0,
		B = 1,
		X = 2,
		Y = 3,
		LeftBumper = 4,
		RightBumper = 5,
		Back = 6,
		Start = 7,
		Guide = 8,
		LeftThumb = 9,
		RightThumb = 10,
		DPadUp = 11,
		DPadRight = 12,
		DPadDown = 13,
		DPadLeft = 14,
		Last = DPadLeft,

		Cross = A,
		Circle = B,
		Square = X,
		Triangle = Y
	};

	enum class GamepadAxis : uint8_t
	{
		LeftX = 0,
		LeftY = 1,
		RightX = 2,
		RightY = 3,
		
		LeftTrigger = 4,
		RightTrigger = 5,
		Last = RightTrigger
	};



	struct GamepadState
	{
	public:
		std::array<JoyStickState, 15> Buttons;
		std::array<float, 6> Axes;
	};

}
