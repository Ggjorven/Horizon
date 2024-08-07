#include "hzpch.h"
#include "Logging.hpp"

#include <Pulse/Core/Logging.hpp>

namespace Hz
{

	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Log::s_Sink = nullptr;
	std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;

	static void PulseLogCallback(Pulse::LogLevel level, std::string message)
	{
		if (level == Pulse::LogLevel::None) return;

		Log::LogMessage((Hz::Log::Level)((uint8_t)level - 1), message);
	}

	static void PulseAssertCallback(bool success, std::string message)
	{
		HZ_ASSERT(success, message);
	}



	void Log::Init()
	{
		s_Sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		s_Sink->set_pattern("[%H:%M:%S] [%L]: %v%$");

		s_Logger = std::make_shared<spdlog::logger>("CherryTree Logger", s_Sink);
		spdlog::set_default_logger(s_Logger);
		spdlog::set_level(spdlog::level::trace);

		// Initialize Pulse
		Pulse::Logger::Init(&PulseLogCallback, &PulseAssertCallback);
	}

	std::shared_ptr<spdlog::logger>& Log::GetLogger()
	{
		return s_Logger;
	}

}
