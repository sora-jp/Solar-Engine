#pragma once

#include "Common.h"
#include "Log.h"
#include <cstdlib>

#ifndef SOLAR_ENABLE_ASSERTS
#define SOLAR_ENABLE_ASSERTS 1
#endif

#define SOLAR_CRITICAL_DETAILS() SOLAR_CRITICAL("    At {}", __FUNCTION__ )
#define SOLAR_CORE_CRITICAL_DETAILS() SOLAR_CORE_CRITICAL("    At {}", __FUNCTION__ )

#define SOLAR_INTERNAL_ASSERT(x, msg) if (!(x)) {SOLAR_CRITICAL(msg, #x); SOLAR_CRITICAL_DETAILS(); std::abort();}
#define SOLAR_INTERNAL_CORE_ASSERT(x, msg) if (!(x)) {SOLAR_CORE_CRITICAL(msg, #x); SOLAR_CORE_CRITICAL_DETAILS(); std::abort();}

#define SOLAR_ASSERT_ALWAYS(x) SOLAR_INTERNAL_ASSERT(x, "Assertion failed on expression \"{}\"")
#define SOLAR_CORE_ASSERT_ALWAYS(x) SOLAR_INTERNAL_CORE_ASSERT(x, "Assertion failed on expression \"{}\"")

#if SOLAR_ENABLE_ASSERTS == 1

#define SOLAR_ASSERT(x) SOLAR_ASSERT_ALWAYS(x)

#else

#define SOLAR_ASSERT(x) (void)0;

#endif

#if defined(SOLAR_ENGINE_BUILD) && SOLAR_ENABLE_ASSERTS == 1

#define SOLAR_CORE_ASSERT(x) SOLAR_CORE_ASSERT_ALWAYS(x)

#else

#define SOLAR_CORE_ASSERT(x) (void)0;

#endif