#include "tfpch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace TheFoolEngine {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_CoreClientLogger;

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("TheFoolEngine");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_CoreClientLogger = spdlog::stdout_color_mt("APP");
		s_CoreClientLogger->set_level(spdlog::level::trace);
	}
}