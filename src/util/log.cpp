#include "log.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::Init(spdlog::level::level_enum logLevel) {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	s_Logger = spdlog::stdout_color_mt("SUNSET");
	s_Logger->set_level(logLevel);
}
