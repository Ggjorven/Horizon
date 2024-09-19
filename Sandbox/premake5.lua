MacOSVersion = MacOSVersion or "14.5"

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
            "%{Dependencies.Vulkan.Linux.LibDir}/%{Dependencies.ShaderC.Linux.LibName}",
		}

    filter "system:macosx"
		defines "HZ_PLATFORM_MACOS"
		systemversion(MacOSVersion)
		staticruntime "on"

		includedirs
		{
			"%{Dependencies.Vulkan.MacOS.IncludeDir}",
		}

		libdirs
		{
			"%{Dependencies.Vulkan.MacOS.LibDir}",
		}

		links
		{
			"%{Dependencies.Vulkan.MacOS.LibName}",
			"%{Dependencies.ShaderC.MacOS.LibName}",

			"AppKit.framework",
			"IOKit.framework",
			"CoreGraphics.framework",
			"CoreFoundation.framework",
		}

		postbuildcommands
		{
			'{COPYFILE} "%{Dependencies.Vulkan.MacOS.LibDir}/libvulkan.1.dylib" "%{cfg.targetdir}"',
			'{COPYFILE} "%{Dependencies.Vulkan.MacOS.LibDir}/lib%{Dependencies.Vulkan.MacOS.LibName}.dylib" "%{cfg.targetdir}"',
		}

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
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

			"%{Dependencies.Vulkan.MacOS.IncludeDir}",
		}

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
		optimize "on"
