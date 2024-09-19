MacOSVersion = MacOSVersion or "14.5"

project "Horizon"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	architecture "x86_64"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	-- Note: VS2022/Make only need the pchheader filename
	pchheader "hzpch.h"
	pchsource "src/Horizon/hzpch.cpp"

	files
	{
		"src/Horizon/**.h",
		"src/Horizon/**.hpp",
		"src/Horizon/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"src",
		"src/Horizon",

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
		"%{Dependencies.glfw.LibName}",
		"%{Dependencies.assimp.LibName}",
		"%{Dependencies.Tracy.LibName}",
		"%{Dependencies.Pulse.LibName}",
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
			"%{Dependencies.Vulkan.Windows.IncludeDir}"
		}

		links
		{
			"%{Dependencies.Vulkan.Windows.LibDir}/%{Dependencies.Vulkan.Windows.LibName}",
			"%{Dependencies.Vulkan.Windows.LibDir}/%{Dependencies.ShaderC.Windows.LibName}",
		}

	filter "system:linux"
		defines "HZ_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

		includedirs
		{
			"%{Dependencies.Vulkan.Linux.IncludeDir}"
		}

		links
		{
			"%{Dependencies.Vulkan.Linux.LibDir}/%{Dependencies.Vulkan.Linux.LibName}",
            "%{Dependencies.Vulkan.Linux.LibDir}/%{Dependencies.ShaderC.Linux.LibName}",

			"Xrandr", "Xi", "GLU", "GL", "X11", "dl", "pthread", "stdc++fs"
		}

    filter "system:macosx"
		defines "HZ_PLATFORM_MACOS"
		systemversion(MacOSVersion)
		staticruntime "on"

		-- Note: XCode only needs the full pchheader path
		pchheader "src/Horizon/hzpch.h"

		includedirs
		{
			"%{Dependencies.Vulkan.MacOS.IncludeDir}"
		}

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"src",
			"src/Horizon",

			"%{Dependencies.spdlog.IncludeDir}",
			"%{Dependencies.glfw.IncludeDir}",
			"%{Dependencies.glm.IncludeDir}",
			"%{Dependencies.stb.IncludeDir}",
			"%{Dependencies.assimp.IncludeDir}",
			"%{Dependencies.Pulse.IncludeDir}",
			"%{Dependencies.Tracy.IncludeDir}",

			"%{Dependencies.Vulkan.MacOS.IncludeDir}",
		}

	-- TODO: Expand
	filter "system:android"
		defines "HZ_PLATFORM_ANDROID"

		includedirs
		{
			-- CMake specific include directories
			"${ANDROID_NDK}/sources/vulkan/include",
            "$<TARGET_PROPERTY:game-activity::game-activity_static,INTERFACE_INCLUDE_DIRECTORIES>"
		}

		defines
		{
			"VK_USE_PLATFORM_ANDROID_KHR"
		}

		-- Android premake options
		androiddependencies
        {
            "androidx.activity:activity:1.9.2",
            "androidx.core:core:1.13.1",
            "androidx.appcompat:appcompat:1.7.0",
            "com.google.android.material:material:1.12.0",
            "androidx.games:games-activity:3.0.5",
        }

		androidabis
        {
            'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }

        androidfindcmakepackages
        {
            "game-activity REQUIRED CONFIG"
        }

        androidlinkcmakepackages
        {
            "game-activity::game-activity_static"
        }

        androidmoduleversions
        { 
            "org.jetbrains.kotlin:kotlin-stdlib:1.8.22", 
            "org.jetbrains.kotlin:kotlin-stdlib-jdk7:1.8.22", 
            "org.jetbrains.kotlin:kotlin-stdlib-jdk8:1.8.22" 
        }

        androidprefab "true"

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
		optimize "Full"
