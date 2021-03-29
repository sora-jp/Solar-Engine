#pragma once

#include "Common.h"
#include "TypeRegistry.h"
#include "Scene.h"
#include <tuple>
#include <utility>

class SOLAR_API BaseSystem
{
public:
	virtual ~BaseSystem() = default;
	void ExecuteInScene(Shared<Scene> scene);
	virtual void ExecuteInScene(Shared<Scene> scene, entt::registry& reg) = 0;
};

inline void BaseSystem::ExecuteInScene(Shared<Scene> scene)
{
	ExecuteInScene(scene, scene->m_registry);
}

template <typename... TComponents> class System : public BaseSystem
{
	template<class ...Args> void ExecMiddleman(Shared<Scene> scene, entt::entity e, Args... args)
	{
	}
	
public:
	System() = default;
	~System() override = default;
	
	void ExecuteInScene(Shared<Scene> scene, entt::registry& reg) override
	{
		auto& view = reg.view<TComponents...>();
		for (auto entRaw : view) 
		{
			std::tuple<TComponents...> components = view.get(entRaw);
			//ExecMiddleman<TComponents...>(scene, entRaw, std::get<TComponents>(components)...);
			Execute(Entity(entRaw, scene), std::get<TComponents, TComponents...>(components)...);
			//std::apply(&System::ExecMiddleman, std::tuple_cat(scene, components));
		}
	}
	
	virtual void Execute(Entity e, TComponents&... components) = 0;
};

INSTANTIATE_FACTORY_DEF(BaseSystem)
#define REGISTER_SYSTEM(sys) REGISTER(BaseSystem, sys)
#define GET_SYSTEMS() GET(BaseSystem)