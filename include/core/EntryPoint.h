#pragma once
#include "Common.h"
#include "Log.h"
#include "Engine.h"
#include "BuildPlatform.h"
#include "SolarApp.h"
#include <iostream>
#include "System.h"

#ifdef SOLAR_PLATFORM_WINDOWS

#define REGISTER_APPLICATION(appClass) SolarApp* CreateApp() { return new appClass(); }
extern SolarApp* CreateApp();
//extern void RegSystems(std::vector<BaseSystem*>&);

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
void main(int argc, char** argv)
{
	const auto app = CreateApp();
	if (!Engine::Init(app))
	{
		SOLAR_CRITICAL("Failed to initialize engine");
	}

	Engine::RegisterEcsSystems(GET_SYSTEMS());
	Engine::Run(app);
	Engine::Shutdown(app);
}

#endif