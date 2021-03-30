#pragma once

#include "Common.h"
#include "Log.h"
#include <cstdlib>

#ifndef SOLAR_ENABLE_ASSERTS
#define SOLAR_ENABLE_ASSERTS 1
#endif

#ifndef __VA_OPT__
#define __VA_OPT__(...) __VA_ARGS__
#endif

#define SOLAR_CRITICAL_DETAILS() SOLAR_CRITICAL("    At {}", __FUNCTION__ )
#define SOLAR_CORE_CRITICAL_DETAILS() SOLAR_CORE_CRITICAL("    At {}", __FUNCTION__ )

#define SOLAR_INTERNAL_ASSERT(x, msg, ...) do {if (x) break; SOLAR_CRITICAL(msg, #x, ##__VA_ARGS__); SOLAR_CRITICAL_DETAILS(); std::abort();} while(0)
#define SOLAR_INTERNAL_CORE_ASSERT(x, msg, ...) do {if (x) break; SOLAR_CORE_CRITICAL(msg, #x, ##__VA_ARGS__); SOLAR_CORE_CRITICAL_DETAILS(); std::abort();} while(0)

#define SOLAR_INTERNAL_DIE(msg, ...) do {SOLAR_CRITICAL(msg, ##__VA_ARGS__); SOLAR_CRITICAL_DETAILS(); std::abort();} while(0)
#define SOLAR_INTERNAL_CORE_DIE(msg, ...) do {SOLAR_CORE_CRITICAL(msg, ##__VA_ARGS__); SOLAR_CORE_CRITICAL_DETAILS(); std::abort();} while(0)

#define SOLAR_ASSERT_ALWAYS(x, ...) SOLAR_INTERNAL_ASSERT(x, "Assertion failed on expression \"{}\"", ##__VA_ARGS__)
#define SOLAR_CORE_ASSERT_ALWAYS(x, ...) SOLAR_INTERNAL_CORE_ASSERT(x, "Assertion failed on expression \"{}\"", ##__VA_ARGS__)

#define SOLAR_DIE(msg, ...) SOLAR_INTERNAL_DIE(msg, ##__VA_ARGS__)
#define SOLAR_CORE_DIE(msg, ...) SOLAR_INTERNAL_CORE_DIE(msg, ##__VA_ARGS__)

#if SOLAR_ENABLE_ASSERTS == 1

#define SOLAR_ASSERT(x, ...) SOLAR_ASSERT_ALWAYS(x, ##__VA_ARGS__)

#else

#define SOLAR_ASSERT(...) (void)0

#endif

#if defined(SOLAR_ENGINE_BUILD) && SOLAR_ENABLE_ASSERTS == 1

#define SOLAR_CORE_ASSERT(x, ...) SOLAR_CORE_ASSERT_ALWAYS(x, ##__VA_ARGS__)

#else

#define SOLAR_CORE_ASSERT(...) (void)0

#endif