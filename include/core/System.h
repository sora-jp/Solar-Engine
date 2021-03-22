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
	template<class Tup, size_t... S> void ExecMiddleman(Shared<Scene> scene, entt::entity e, Tup components, std::index_sequence<S ...>)
	{
		Execute(Entity(e, scene), std::get<S>(components)...);
	}
	
public:
	System() = default;
	~System() override = default;
	
	void ExecuteInScene(Shared<Scene> scene, entt::registry& reg) override
	{
		auto& view = reg.view<TComponents...>();
		for (auto entRaw : view) 
		{
			auto components = view.get(entRaw);
			ExecMiddleman(scene, entRaw, components, std::make_index_sequence<sizeof...(TComponents)>{});
			//std::apply(&System::ExecMiddleman, std::tuple_cat(scene, components));
		}
	}
	
	virtual void Execute(Entity e, TComponents&... components) = 0;
};

INSTANTIATE_FACTORY(BaseSystem);

#define REGISTER_SYSTEM(sys) REGISTER(TypeRegistry<BaseSystem>, sys)
#define GET_SYSTEMS() GET(TypeRegistry<BaseSystem>)