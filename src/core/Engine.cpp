#include "pch.h"
#include "Engine.h"
#include "Common.h"
#include "Subsystem.h"
#include "SolarApp.h"
#include <Windows.h>
#include "System.h"
#include "Profiler.h"

std::vector<Shared<Subsystem>> Engine::_subsystems;
std::vector<Shared<BaseSystem>> Engine::_ecssystems;

bool Engine::Init(SolarApp* app)
{
	SOLAR_CORE_TRACE("Initializing engine");
	SOLAR_CORE_ASSERT(app != nullptr);

	app->UseSubsystems();

	for (const auto& sys : _subsystems)
	{
		SOLAR_CORE_ASSERT(sys != nullptr);

		sys->Init();
		SOLAR_CORE_TRACE("Loaded subsystem \"{}\"", sys->GetName());
	}

	std::stable_sort(_subsystems.begin(), _subsystems.end());
	app->Init();
	SOLAR_CORE_INFO("Initialized app \"{}\"", app->GetName());

	return true;
}

inline void Engine::RunEcsSystem(const Shared<BaseSystem>& sys)
{
	for (auto& scene : Scene::GetLoadedScenes()) sys->ExecuteInScene(scene);
}

void Engine::Run(SolarApp* app)
{
	SOLAR_CORE_ASSERT(app != nullptr);
	
	for (auto& ptr : _ecssystems)
	{
		SOLAR_CORE_TRACE("ECS system at {}", static_cast<void*>(&ptr));
	}
	
	while (true)
	{
		Profiler::BeginRoot();
		
		Profiler::Begin("PreRun", "Engine");
		foreach_reverse(_subsystems, &Subsystem::PreRun);
		Profiler::End();

		Profiler::Begin("App", "App");
		foreach(_ecssystems, RunEcsSystem);
		app->Run();
		Profiler::End();

		Profiler::Begin("PostRun", "Engine");
		foreach(_subsystems, &Subsystem::PostRun);
		if (any(_subsystems, &Subsystem::RequestedShutdown)) break;
		Profiler::End();
		Profiler::EndRoot();
	}
}

void Engine::Shutdown(SolarApp* app)
{
	SOLAR_CORE_TRACE("Engine shutdown");
	SOLAR_CORE_ASSERT(app != nullptr);
	
	app->Shutdown();

	for (auto& sys : _subsystems)
	{
		SOLAR_CORE_ASSERT(sys != nullptr);
		SOLAR_CORE_TRACE("Unloading subsystem \"{}\"", sys->GetName());
		sys->Shutdown();
		sys.reset();
	}

	delete app;
	_subsystems.clear();
}
