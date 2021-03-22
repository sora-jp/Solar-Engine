#include "pch.h"
#include "Engine.h"
#include "Common.h"
#include "Subsystem.h"
#include "SolarApp.h"
#include <Windows.h>
#include "System.h"

namespace fs = std::filesystem;

#define GET_CREATE_SUBSYSTEM_PROC(hmod) CreateSubsystemFunc(GetProcAddress(hmod, "CreateSubsystem"))
#define CREATE_SUBSYSTEM(hmod, ctor) std::shared_ptr<Subsystem>(ctor())

std::vector<SubsystemPtr<Subsystem>> Engine::_subsystems;
std::vector<Shared<BaseSystem>> Engine::_ecssystems;

bool Engine::Init(SolarApp* app)
{
	SOLAR_CORE_TRACE("Initializing engine");
	SOLAR_CORE_ASSERT(app != nullptr);
	_subsystems = std::vector<SubsystemPtr<Subsystem>>();

	// TODO: Platform independent way of doing this
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileName(nullptr, path, MAX_PATH);
	
	for (const auto& fsEntry : fs::directory_iterator(fs::path(path).parent_path()))
	{
		if (fsEntry.path().extension() != ".dll") continue;
		if (fsEntry.path().filename() == "Core.dll") continue;

		const auto instance = LoadLibrary(fsEntry.path().c_str());
		const auto ctor = GET_CREATE_SUBSYSTEM_PROC(instance);
		if (ctor == nullptr)
		{
			SOLAR_CORE_TRACE("Library {} did not contain a subsystem", fsEntry.path().filename().string());
			FreeLibrary(instance);
			continue;
		}
		
		auto sys = CREATE_SUBSYSTEM(instance, ctor);
		SOLAR_CORE_ASSERT(sys != nullptr);

		sys->Init();
		SOLAR_CORE_TRACE("Loaded subsystem \"{}\" ({})", sys->GetName(), fsEntry.path().filename().string());

		_subsystems.push_back(sys);
	}

	std::stable_sort(_subsystems.begin(), _subsystems.end());
	app->Init();
	SOLAR_CORE_INFO("Initialized app \"{}\"", app->GetName());

	return true;
}

void Engine::RegisterEcsSystems(const std::vector<Shared<BaseSystem>>& systems)
{
	SOLAR_CORE_TRACE("Registering {} ECS systems", systems.size());
	std::copy(systems.cbegin(), systems.cend(), std::back_inserter(_ecssystems));
	//for (auto sys : (std::make_move_iterator(systems.begin()), std::make_move_iterator(systems.end()))) _ecssystems.push_back(sys);
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
		foreach_reverse(_subsystems, &Subsystem::PreRun);
		
		app->Run();
		foreach(_ecssystems, RunEcsSystem);
		
		foreach(_subsystems, &Subsystem::PostRun);
		if (any(_subsystems, &Subsystem::RequestedShutdown)) break;
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
