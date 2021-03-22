#include "pch.h"
#include "Scene.h"
#include "Entity.h"
#include "TransformComponent.h"

std::vector<Shared<Scene>> Scene::_loadedScenes;

//Scene::Scene()
//{
//	SOLAR_CORE_TRACE("Scene::Scene()");
//}
//
//Scene::~Scene()
//{
//	SOLAR_CORE_TRACE("Scene::~Scene()");
//}

Shared<Scene> Scene::Create()
{
	auto ptr = MakeShared<Scene>();
	_loadedScenes.push_back(ptr);
	return ptr;
}

void Scene::Destroy()
{
	std::remove(_loadedScenes.begin(), _loadedScenes.end(), shared_from_this());
}

const std::vector<Shared<Scene>>& Scene::GetLoadedScenes()
{
	return _loadedScenes;
}

Entity Scene::CreateEntity(const std::string& name = std::string(), const Entity& parent = { entt::null, nullptr })
{
	Entity e = { m_registry.create(), shared_from_this() };
	e.AddComponent<TransformComponent>();
	e.AddComponent<CommonEntityData>(e, name, parent);
	return e;
}
