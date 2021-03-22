#pragma once
#include "Common.h"

class SOLAR_API SolarApp
{
public:
	SolarApp() = default;
	virtual ~SolarApp() = default;

	virtual void Init() = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	[[nodiscard]] virtual const char* GetName() const { return "New Solar App"; }
};

#define SOLAR_APPNAME(name) const char* GetName() const override { return name; }