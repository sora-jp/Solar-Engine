#include "pch.h"
#include "Log.h"

#pragma warning(push, 0)
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#pragma warning(pop)

std::vector<spdlog::sink_ptr> Log::_sinks;
Shared<spdlog::logger> Log::_logger = nullptr;
Shared<spdlog::logger> Log::_coreLogger = nullptr;

inline void ConfigureLogger(const Shared<spdlog::logger>& logger)
{
	register_logger(logger);
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
}

void Log::EnsureCreated()
{
	if (_logger == nullptr)
	{
		spdlog::set_level(spdlog::level::trace);

		_sinks = std::vector<spdlog::sink_ptr>();
		_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		_sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Solar.log", false));

		_sinks[0]->set_pattern("%^[ %-5n %-8l ] %-70v (%s:%#)%$");
		_sinks[1]->set_pattern("[%-8T] [ %-5n %-8l ] %-128v (thread %t, %s:%#)");

		_logger = MakeShared<spdlog::logger>("app", _sinks.begin(), _sinks.end());
		_coreLogger = MakeShared<spdlog::logger>("solar", _sinks.begin(), _sinks.end());
		
		ConfigureLogger(_logger);
		ConfigureLogger(_coreLogger);

		SOLAR_CORE_TRACE("Logging initialized");
	}
}