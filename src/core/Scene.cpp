#include "pch.h"
#include "Scene.h"
#include "TransformComponent.h"

std::vector<Shared<Scene>> Scene::_loadedScenes;

Shared<Scene> Scene::Create()
{
	auto ptr = MakeShared<Scene>();
	_loadedScenes.push_back(ptr);
	return ptr;
}

void Scene::Destroy()
{
	_loadedScenes.erase(std::remove(_loadedScenes.begin(), _loadedScenes.end(), shared_from_this()));
}

const std::vector<Shared<Scene>>& Scene::GetLoadedScenes()
{
	return _loadedScenes;
}

void Scene::IterateTopLevelEntities(const std::function<void(const Entity&&, const CommonEntityData&)>& func)
{
	const auto es = m_registry.view<const CommonEntityData>();
	for (auto&& [e, data] : es.each())
	{
		if (data.GetParent() != Entity::null) continue;
		func({e, shared_from_this()}, data);
	}
}

Entity Scene::CreateEntity(const std::string& name = std::string(), const Entity& parent = { entt::null, nullptr })
{
	Entity e = { m_registry.create(), shared_from_this() };
	e.AddComponent<TransformComponent>();
	e.AddComponent<CommonEntityData>(e, name, parent);
	return e;
}
