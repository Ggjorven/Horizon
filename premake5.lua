-- require("vendor/premake-vscode/vscode")

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
VULKAN_SDK = os.getenv("VULKAN_SDK")

Dependencies =
{
	-- Vulkan Related
	Vulkan =
	{
		Windows =
		{
			LibName = "vulkan-1",
			IncludeDir = "%{VULKAN_SDK}/Include/",
			LibDir = "%{VULKAN_SDK}/Lib/"
		},
		Linux =
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/include/",
			LibDir = "%{VULKAN_SDK}/lib/"
		}
	},
	ShaderC =
	{
		LibName = "shaderc_shared"
	},
	VMA =
	{
		LibName = "VMA",
		IncludeDir = "%{wks.location}/vendor/vma/include"
	},

	-- All other Third-Party libraries
	spdlog =
	{
		IncludeDir = "%{wks.location}/vendor/spdlog/include"
	},
	glfw =
	{
		LibName = "GLFW",
		IncludeDir = "%{wks.location}/vendor/glfw/include"
	},
	glm =
	{
		IncludeDir = "%{wks.location}/vendor/glm"
	},
	Pulse =
	{
		LibName = "Pulse",
		IncludeDir = "%{wks.location}/vendor/Pulse/Pulse/src"
	}
}
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Solution
------------------------------------------------------------------------------
outputdir = "%{cfg.buildcfg}-%{cfg.system}"

workspace "Horizon"
	architecture "x86_64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "Dependencies"
	include "vendor/glfw"
	include "vendor/Pulse/Pulse"
	include "vendor/vma"
group ""

group "Horizon"
	include "Horizon"
group ""

include "Sandbox"
------------------------------------------------------------------------------
