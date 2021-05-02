#pragma once
#include <vector>

#include "Common.h"
#include "Subsystem.h"
#include "SolarApp.h"
#include "System.h"

class SOLAR_API Engine final
{
	static std::vector<Shared<Subsystem>> _subsystems;
	static std::vector<Shared<BaseSystem>> _ecssystems;
	
public:
	template<class T, std::enable_if_t<std::is_base_of_v<Subsystem, T>, bool> = true>
	static void UseSubsystem()
	{
		_subsystems.push_back(MakeShared<T>());
	}

	template<class T, std::enable_if_t<std::is_base_of_v<BaseSystem, T>, bool> = true>
	static void UseSystem()
	{
		_ecssystems.push_back(MakeShared<T>());
	}
	
	static bool Init(SolarApp* app);
	static void RunEcsSystem(const Shared<BaseSystem>& sys);
	static void Run(SolarApp* app);
	static void Shutdown(SolarApp* app);
};
