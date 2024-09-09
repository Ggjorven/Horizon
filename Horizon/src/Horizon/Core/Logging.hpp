#pragma once

#include "Horizon/Core/Core.hpp"

#include <string>
#include <string_view>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Pulse/Core/Core.hpp>

namespace Hz
{

	class Log
	{
	public:
		enum class Level : uint8_t
		{
			None = 0, Trace, Info, Warn, Error, Fatal
		};

		static void Init();

        template<typename ... Args>
		static void LogMessage(Log::Level level, std::string_view fmt, const Args& ...args);

		#if !defined(HZ_CONFIG_DIST)
			#define HZ_LOG_TRACE(...)	::Hz::Log::LogMessage(Hz::Log::Level::Trace, __VA_ARGS__);
			#define HZ_LOG_INFO(...)	::Hz::Log::LogMessage(Hz::Log::Level::Info, __VA_ARGS__);
			#define HZ_LOG_WARN(...)	::Hz::Log::LogMessage(Hz::Log::Level::Warn, __VA_ARGS__);
			#define HZ_LOG_ERROR(...)	::Hz::Log::LogMessage(Hz::Log::Level::Error, __VA_ARGS__);
			#define HZ_LOG_FATAL(...)	::Hz::Log::LogMessage(Hz::Log::Level::Fatal, __VA_ARGS__);
		#else
			#define HZ_LOG_TRACE(...)
			#define HZ_LOG_INFO(...)
			#define HZ_LOG_WARN(...)
			#define HZ_LOG_ERROR(...)
			#define HZ_LOG_FATAL(...)
		#endif

		static std::shared_ptr<spdlog::logger>& GetLogger();

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
		static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_ConsoleSink;
	};

#if defined(HZ_DESKTOP_ENVIRONMENT)
    template<typename ... Args>
	void Log::LogMessage(Log::Level level, std::string_view fmt, const Args& ...args)
	{
		switch (level)
		{
		case Level::Trace:
			spdlog::trace(Text::Format(fmt, args...));
			break;
		case Level::Info:
			spdlog::info(Text::Format(fmt, args...));
			break;
		case Level::Warn:
			spdlog::warn(Text::Format(fmt, args...));
			break;
		case Level::Error:
			spdlog::error(Text::Format(fmt, args...));
			break;
		case Level::Fatal:
			spdlog::critical(Text::Format(fmt, args...));
			break;
		}
	}
#elif defined(HZ_PLATFORM_ANDROID)
	// TODO: ...
#endif

	#ifndef HZ_CONFIG_DIST
		#define HZ_VERIFY(value, ...) if (!(value)) \
			{ \
				HZ_LOG_FATAL(__VA_ARGS__); \
			}
	#else
		#define HZ_VERIFY(value, ...)
	#endif

	#ifdef HZ_CONFIG_DEBUG
        // Note: Don't put function calls in HZ_ASSERT! They don't run in DIST
		#define HZ_ASSERT(value, ...) if (!(value)) \
			{ \
				HZ_LOG_FATAL(__VA_ARGS__); \
				PULSE_DEBUG_BREAK(); \
			}
	#elif defined(HZ_CONFIG_RELEASE)
        // Note: Don't put function calls in HZ_ASSERT! They don't run in DIST
		#define HZ_ASSERT(value, ...) if (!(value)) \
			{ \
				HZ_LOG_FATAL(__VA_ARGS__); \
			}
	#else
        // Note: Don't put function calls in HZ_ASSERT! They don't run in DIST
		#define HZ_ASSERT(value, ...)
	#endif

}
