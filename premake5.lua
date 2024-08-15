require("vendor/premake-vscode/vscode")

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
		},
        MacOS = -- TODO: Implement -- Note: We expect VULKAN_SDK to be /Users/XXX/VulkanSDK/XVersionX/ (macOS or iOS)
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/"
		},
        iOS = -- Not supported -- Note: We expect VULKAN_SDK to be /Users/XXX/VulkanSDK/XVersionX/ (macOS or iOS)
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/../iOS/include/",
			LibDir = "%{VULKAN_SDK}/../iOS/lib/"
		}
	},
	ShaderC =
	{
		LibName = "shaderc_shared"
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
    stb =
	{
		IncludeDir = "%{wks.location}/vendor/stb"
	},
	Pulse =
	{
		LibName = "Pulse",
		IncludeDir = "%{wks.location}/vendor/Pulse/Pulse/src"
	},
    Tracy =
    {
        LibName = "Tracy",
		IncludeDir = "%{wks.location}/vendor/tracy/tracy/public"
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
    include "vendor/tracy"
	include "vendor/Pulse/Pulse"
group ""

group "Horizon"
	include "Horizon"
group ""

include "Sandbox"
------------------------------------------------------------------------------
