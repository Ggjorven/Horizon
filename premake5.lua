require("vendor/premake-vscode/vscode")

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
VULKAN_SDK = os.getenv("VULKAN_SDK")
VULKAN_VERSION = VULKAN_SDK:match("(%d+%.%d+%.%d+)") -- Example: 1.3.290 (without the 0)

MacOSVersion = "14.5"

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
        MacOS = 	-- Note: Vulkan on MacOS is currently dynamic. (Example: libvulkan1.3.290.dylib)
		{
			LibName = "vulkan.%{VULKAN_VERSION}",
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/",
		},
		Android = 	-- Unsupported.
		{
			LibName = "vulkan-1",
			IncludeDir = "%{VULKAN_SDK}/Include/",
			LibDir = "%{VULKAN_SDK}/Lib/"
		},
        iOS = 		-- Unsupported.
		{
			LibName = "vulkan.%{VULKAN_VERSION}", 
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/",
		}
	},
	ShaderC =
	{
		Windows = { LibName = "shaderc_shared" },
		Linux = { LibName = "shaderc_shared" },
		MacOS = { LibName = "shaderc_combined" },
		iOS = { LibName = "shaderc_combined" },
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
    assimp =
	{
        LibName = "Assimp",
		IncludeDir = "%{wks.location}/vendor/assimp/include"
	},
	Pulse =
	{
		LibName = "Pulse",
		IncludeDir = "%{wks.location}/vendor/Pulse/Pulse/src"
	},
    Tracy =
    {
        LibName = "Tracy",
		IncludeDir = "%{wks.location}/vendor/tracy/public"
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
    include "vendor/assimp"
    include "vendor/tracy"
	include "vendor/Pulse/Pulse"
group ""

group "Horizon"
	include "Horizon"
group ""

include "Sandbox"
------------------------------------------------------------------------------
