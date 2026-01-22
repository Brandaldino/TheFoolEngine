#pragma once



#include "Base.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"



namespace TheFoolEngine {

	class Log{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetCoreClientLogger() { return s_CoreClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_CoreClientLogger;
	};

}

// Core Log Macros
#define TF_CORE_TRACE(...)		::TheFoolEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TF_CORE_INFO(...)		::TheFoolEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TF_CORE_WARN(...)		::TheFoolEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TF_CORE_ERROR(...)		::TheFoolEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TF_CORE_FATAL(...)		::TheFoolEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)


// Client Log Macros
#define TF_TRACE(...)			::TheFoolEngine::Log::GetCoreClientLogger()->trace(__VA_ARGS__)
#define TF_INFO(...)			::TheFoolEngine::Log::GetCoreClientLogger()->info(__VA_ARGS__)
#define TF_WARN(...)			::TheFoolEngine::Log::GetCoreClientLogger()->warn(__VA_ARGS__)
#define TF_ERROR(...)			::TheFoolEngine::Log::GetCoreClientLogger()->error(__VA_ARGS__)
#define TF_FATAL(...)			::TheFoolEngine::Log::GetCoreClientLogger()->fatal(__VA_ARGS__)
