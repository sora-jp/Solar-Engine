#include "pch.h"
#include "Entity.h"

Entity::Entity(const entt::entity handle, const Shared<Scene>& scene) : m_handle(handle), m_scene(scene) {}

const Entity&& Entity::null = {entt::null, nullptr};