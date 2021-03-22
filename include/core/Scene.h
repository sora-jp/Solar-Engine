#pragma once
#include "Common.h"
#include "entt/entt.hpp"

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

	Entity CreateEntity(const std::string& name, const Entity& parent );
};