#pragma once

#include "Horizon/Core/Core.hpp"

#include <cstdint>
#include <string>
#include <filesystem>
#undef CreateDirectory // This has to be done for windows to work.

namespace Hz::IO
{

	// Note: The filesystem class is implemented based on platform
	class FileSystem
	{
	public:
		static bool Exists(const std::filesystem::path& path);

		static bool CreateDirectory(const std::filesystem::path& path);

		static bool Remove(const std::filesystem::path& path);
		static void Rename(const std::filesystem::path& oldName, const std::filesystem::path& newName);

		static std::filesystem::path Absolute(const std::filesystem::path& path);
		static std::filesystem::path Relative(const std::filesystem::path& path, const std::filesystem::path& base = CurrentPath());
		
		static std::filesystem::path CurrentPath();
	};

	enum class FileMode
	{
		In = 1 << 1,
		Out = 1 << 2,
		Ate = 1 << 3,
		App = 1 << 4,
		Trunc = 1 << 5,
		Binary = 1 << 6,
	};
	ENABLE_BITWISE(FileMode)

}

// For templated functions and classes
#if defined(HZ_PLATFORM_WINDOWS)
	#include "Horizon/Platform/Windows/WindowsFileSystem.hpp"
#elif defined(HZ_PLATFORM_LINUX)
	#include "Horizon/Platform/Linux/LinuxFileSystem.hpp"
#elif defined(HZ_PLATFORM_MACOS)
	#include "Horizon/Platform/MacOS/MacOSFileSystem.hpp"
#elif defined(HZ_PLATFORM_ANDROID)
	#error TODO: Implement Android FileSystem
#endif