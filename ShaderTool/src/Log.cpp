#include "pch.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::s_coreLogger;
		
void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	s_coreLogger = spdlog::stdout_color_mt("LOG");
	s_coreLogger->set_level(spdlog::level::trace);
}
