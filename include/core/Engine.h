#pragma once
#include <vector>

#include "Common.h"
#include "Subsystem.h"
#include "SolarApp.h"
#include "System.h"

class SOLAR_API Engine final
{
	static std::vector<SubsystemPtr<Subsystem>> _subsystems;
	static std::vector<Shared<BaseSystem>> _ecssystems;
	
public:
	static bool Init(SolarApp* app);
	static void RegisterEcsSystems(const std::vector<Shared<BaseSystem>>& systems);
	static void RunEcsSystem(const Shared<BaseSystem>& sys);
	static void Run(SolarApp* app);
	static void Shutdown(SolarApp* app);
};
