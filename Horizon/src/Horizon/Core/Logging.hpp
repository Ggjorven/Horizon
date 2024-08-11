#pragma once

#include <string>
#include <string_view>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Pulse/Core/Core.hpp>
#include <Pulse/Text/Format.hpp>

namespace Hz
{

	class Log
	{
	public:
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal, None = 255
		};

		static void Init();

		template<typename ... Args>
		static void LogMessage(Log::Level level, std::string_view fmt, const Args&... args);

		#ifndef HZ_CONFIG_DIST
			#define HZ_LOG_TRACE(...)	Hz::Log::LogMessage(Hz::Log::Level::Trace, __VA_ARGS__);
			#define HZ_LOG_INFO(...)	Hz::Log::LogMessage(Hz::Log::Level::Info, __VA_ARGS__);
			#define HZ_LOG_WARN(...)	Hz::Log::LogMessage(Hz::Log::Level::Warn, __VA_ARGS__);
			#define HZ_LOG_ERROR(...)	Hz::Log::LogMessage(Hz::Log::Level::Error, __VA_ARGS__);
			#define HZ_LOG_FATAL(...)	Hz::Log::LogMessage(Hz::Log::Level::Fatal, __VA_ARGS__);
		#else
			#define HZ_LOG_TRACE(...)
			#define HZ_LOG_INFO(...)
			#define HZ_LOG_WARN(...)
			#define HZ_LOG_ERROR(...)
			#define HZ_LOG_FATAL(...)
		#endif

		static std::shared_ptr<spdlog::logger>& GetLogger();

	private:
		static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_Sink;
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

	template<typename ... Args>
	void Log::LogMessage(Log::Level level, std::string_view fmt, const Args&... args)
	{
		switch (level)
		{
		case Level::Trace:
			spdlog::trace(Pulse::Text::Format(fmt, args...));
			break;
		case Level::Info:
			spdlog::info(Pulse::Text::Format(fmt, args...));
			break;
		case Level::Warn:
			spdlog::warn(Pulse::Text::Format(fmt, args...));
			break;
		case Level::Error:
			spdlog::error(Pulse::Text::Format(fmt, args...));
			break;
		case Level::Fatal:
			spdlog::critical(Pulse::Text::Format(fmt, args...));
			break;
		}
	}

	#ifndef HZ_CONFIG_DIST
		#define HZ_VERIFY(value, ...) if (!value) \
			{ \
				HZ_LOG_FATAL(__VA_ARGS__); \
			}
	#else
		#define HZ_VERIFY(value, ...)
	#endif

	#ifdef HZ_CONFIG_DEBUG
		#define HZ_ASSERT(value, ...) if (!value) \
			{ \
				HZ_LOG_FATAL(__VA_ARGS__); \
				PULSE_DEBUG_BREAK(); \
			}
	#elif defined(HZ_RELEASE)
		#define HZ_ASSERT(value, ...) if (!(value)) \
			{ \
				CT_LOG_FATAL(__VA_ARGS__); \
			}
	#else
		#define HZ_ASSERT(value, ...)
	#endif

}
