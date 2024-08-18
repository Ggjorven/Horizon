project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	architecture "x86_64"

	debugdir ("%{wks.location}")

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
	}

	includedirs
	{
		"src",
		"%{wks.location}/vendor",

		"%{wks.location}/Horizon/src",

		"%{Dependencies.spdlog.IncludeDir}",
		"%{Dependencies.glfw.IncludeDir}",
		"%{Dependencies.glm.IncludeDir}",
		"%{Dependencies.stb.IncludeDir}",
		"%{Dependencies.assimp.IncludeDir}",
		"%{Dependencies.Pulse.IncludeDir}",
		"%{Dependencies.Tracy.IncludeDir}",
	}

	links
	{
		"Horizon"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE"
	}

	filter "system:windows"
		defines "HZ_PLATFORM_WINDOWS"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

		includedirs
		{
			"%{Dependencies.Vulkan.Windows.IncludeDir}",
		}

	filter "system:linux"
		defines "HZ_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

		includedirs
		{
			"%{Dependencies.Vulkan.Linux.IncludeDir}",
		}

		-- Otherwise it doesn't link properly on linux (weird)
		links
		{
			"%{Dependencies.glfw.LibName}",
			"%{Dependencies.assimp.LibName}",
			"%{Dependencies.Tracy.LibName}",
			"%{Dependencies.Pulse.LibName}",

			"%{Dependencies.Vulkan.Linux.LibDir}/%{Dependencies.Vulkan.Linux.LibName}",
            "%{Dependencies.Vulkan.Linux.LibDir}/%{Dependencies.ShaderC.LibName}",
		}

    -- TODO: Properly implement MacOS
    filter "system:macosx"
		defines "HZ_PLATFORM_MACOS"
		systemversion "latest"
		staticruntime "on"

		includedirs
		{
			"%{Dependencies.Vulkan.MacOS.IncludeDir}",
		}

	filter "configurations:Debug"
		defines "HZ_CONFIG_DEBUG"
		runtime "Debug"
		symbols "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Release"
		defines "HZ_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Dist"
		defines "HZ_CONFIG_DIST"
		runtime "Release"
		optimize "on"
