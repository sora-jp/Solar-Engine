#pragma once
#include "Common.h"
#include "entt/entt.hpp"
#include <functional>

class Entity;
class BaseSystem;

class SOLAR_API Scene final : public std::enable_shared_from_this<Scene>
{
	friend class Entity;
	friend class BaseSystem;
	
	entt::registry m_registry;
	static std::vector<Shared<Scene>> _loadedScenes;

public:
	//Scene();
	//~Scene();
	
	static Shared<Scene> Create();
	void Destroy();
	static const std::vector<Shared<Scene>>& GetLoadedScenes();
	template<typename... C, typename F> void IterateEntities(F func);

	Entity CreateEntity(const std::string& name, const Entity& parent );
};

template <typename... C, typename F>
void Scene::IterateEntities(F func)
{
	auto view = m_registry.view<C...>();
	for (auto e : view)
	{
		std::tuple<C...> components = view.template get<C...>(e);
		func(Entity(e, shared_from_this()), std::get<C, C...>(components)...);
	}
}