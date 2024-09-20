#pragma once

#include "Horizon/Core/Core.hpp"

#include <cstdint>

namespace Hz
{

	// From glfw3.h
	enum class CursorMode : uint32_t
	{
		Shown = 0x00034001,
		Hidden = 0x00034002,
		Disabled = 0x00034003,
		Captured = 0x00034004
	};

	// From glfw3.h
	enum class MouseButton : uint8_t
	{
		B1 = 0,
		B2 = 1,
		B3 = 2,
		B4 = 3,
		B5 = 4,
		B6 = 5,
		B7 = 6,
		B8 = 7,

		Last = B7,
		Left = B1,
		Right = B2,
		Middle = B3
	};

}
