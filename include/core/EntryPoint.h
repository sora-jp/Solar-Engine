#pragma once
#include "Common.h"
#include "Log.h"
#include "Engine.h"
#include "BuildPlatform.h"
#include "SolarApp.h"
#include "System.h"

#ifdef SOLAR_PLATFORM_WINDOWS

#define REGISTER_APPLICATION(appClass) SolarApp* CreateApp() { return new appClass(); }
extern SolarApp* CreateApp();
//extern void RegSystems(std::vector<BaseSystem*>&);

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main(int argc, char** argv)
{
	const auto app = CreateApp();
	
	if (!Engine::Init(app))
	{
		SOLAR_CRITICAL("Failed to initialize engine");
		return 1;
	}
	
	Engine::Run(app);
	Engine::Shutdown(app);
	return 0;
}

#endif