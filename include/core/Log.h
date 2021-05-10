#pragma once
#pragma warning(push)
#pragma warning(disable:4251) //TODO: Figure out if this causes any actual problems

#define SPDLOG_HEADER_ONLY
#define ENABLE_CORE_LOG

#if defined(SOLAR_ENGINE_BUILD) || defined(SOLAR_SUBSYSTEM_BUILD)
#define LOG_BUILD_CORE

#ifdef ENABLE_CORE_LOG
#define LOG_INCLUDE_CORE
#endif

#endif

#ifndef LOG_ENABLED
#define LOG_ENABLED 1
#endif

#include "Common.h"

#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#pragma warning(pop)

#define LOG_FN(loggerName, x, lvl) \
	template<typename FormatString, typename... Args> inline static void x(const FormatString& fmt, Args&&... args) { x##_loc(spdlog::source_loc{"unknown", -1, ""}, fmt, std::forward<Args>(args)...); }; \
	template<typename FormatString, typename... Args> inline static void x##_loc(const spdlog::source_loc& loc, const FormatString& fmt, Args&&... args) \
	{ \
		EnsureCreated(); \
		::Log::loggerName->log(loc, spdlog::level::lvl, fmt, std::forward<Args>(args)...); \
	};

class SOLAR_API Log final {
	static std::vector<spdlog::sink_ptr> _sinks;
	static Shared<spdlog::logger> _logger;
	static Shared<spdlog::logger> _coreLogger;
	static void EnsureCreated();

public:
	Log() = delete;

	LOG_FN(_logger, Trace, trace)
	LOG_FN(_logger, Info, info)
	LOG_FN(_logger, Warn, warn)
	LOG_FN(_logger, Error, err)
	LOG_FN(_logger, Critical, critical)

#ifdef LOG_INCLUDE_CORE
	LOG_FN(_coreLogger, CoreTrace, trace)
	LOG_FN(_coreLogger, CoreInfo, info)
	LOG_FN(_coreLogger, CoreWarn, warn)
	LOG_FN(_coreLogger, CoreError, err)
	LOG_FN(_coreLogger, CoreCritical, critical)
#endif
};

#if (LOG_ENABLED == 1)
#define SOLAR_LOG(fn, ...) ::Log::fn##_loc(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#else
#define SOLAR_LOG(...) (void)0
#endif

#define SOLAR_TRACE(str, ...) SOLAR_LOG(Trace, str, __VA_ARGS__)
#define SOLAR_INFO(str, ...) SOLAR_LOG(Info, str, __VA_ARGS__)
#define SOLAR_WARN(str, ...) SOLAR_LOG(Warn, str, __VA_ARGS__)
#define SOLAR_ERROR(str, ...) SOLAR_LOG(Error, str, __VA_ARGS__)
#define SOLAR_CRITICAL(str, ...) SOLAR_LOG(Critical, str, __VA_ARGS__)

#define SOLAR_TRACE_S(str, ...) SOLAR_LOG(Trace, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_INFO_S(str, ...) SOLAR_LOG(Info, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_WARN_S(str, ...) SOLAR_LOG(Warn, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_ERROR_S(str, ...) SOLAR_LOG(Error, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_CRITICAL_S(str, ...) SOLAR_LOG(Critical, FMT_STRING(str), __VA_ARGS__)

#ifdef LOG_INCLUDE_CORE

#define SOLAR_CORE_TRACE(str, ...) SOLAR_LOG(CoreTrace, str, __VA_ARGS__)
#define SOLAR_CORE_INFO(str, ...) SOLAR_LOG(CoreInfo, str, __VA_ARGS__)
#define SOLAR_CORE_WARN(str, ...) SOLAR_LOG(CoreWarn, str, __VA_ARGS__)
#define SOLAR_CORE_ERROR(str, ...) SOLAR_LOG(CoreError, str, __VA_ARGS__)
#define SOLAR_CORE_CRITICAL(str, ...) SOLAR_LOG(CoreCritical, str, __VA_ARGS__)

#define SOLAR_CORE_TRACE_S(str, ...) SOLAR_LOG(CoreTrace, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_CORE_INFO_S(str, ...) SOLAR_LOG(CoreInfo, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_CORE_WARN_S(str, ...) SOLAR_LOG(CoreWarn, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_CORE_ERROR_S(str, ...) SOLAR_LOG(CoreError, FMT_STRING(str), __VA_ARGS__)
#define SOLAR_CORE_CRITICAL_S(str, ...) SOLAR_LOG(CoreCritical, FMT_STRING(str), __VA_ARGS__)

#elif defined(LOG_BUILD_CORE)

#define SOLAR_CORE_TRACE(...) (void)0
#define SOLAR_CORE_INFO(...) (void)0
#define SOLAR_CORE_WARN(...) (void)0
#define SOLAR_CORE_ERROR(...) (void)0
#define SOLAR_CORE_CRITICAL(...) (void)0

#endif

#undef LOG_FN
#undef LOG_INCLUDE_CORE
#pragma warning(pop)