#pragma once
#include "Common.h"
#include "Assert.h"
#include "Scene.h"
#include <type_traits>

#include "entt/entt.hpp"

class SOLAR_API Entity final
{
	friend struct CommonEntityData;
	
	entt::entity m_handle {entt::null};
	Shared<Scene> m_scene;

public:
	static const Entity&& null;
	Entity(entt::entity handle, const Shared<Scene>& scene);
	
	template<class T, typename... Args, std::enable_if_t<std::is_constructible_v<T, Args...>, bool> = true>
	T& AddComponent(Args&&... args)
	{
		SOLAR_ASSERT(!HasComponent<T>());
		return m_scene->m_registry.emplace<T, Args...>(m_handle, std::forward<Args>(args)...);
	}

	template <class T, std::enable_if_t<!std::is_empty_v<T>, bool> = true>
	[[nodiscard]] T& GetComponent() const
	{
		SOLAR_ASSERT(HasComponent<T>());
		return m_scene->m_registry.get<T>(m_handle);
	}

	template <class T> bool RemoveComponent()
	{
		if (!HasComponent<T>()) return false;
		m_scene->m_registry.remove<T>(m_handle);
		return true;
	}

	template <class T> [[nodiscard]] bool HasComponent() const
	{
		return m_scene->m_registry.all_of<T>(m_handle);
	}

	operator bool() const { return m_scene && m_scene->m_registry.valid(m_handle); }
	operator entt::entity() const { return m_handle; }
	operator uint32_t() const { return uint32_t(m_handle); }

	bool operator==(const Entity& other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }
	bool operator!=(const Entity& other) const { return !(*this == other); }
};

struct CommonEntityData
{
private:
	entt::entity m_parent{ entt::null };
	Shared<Scene> m_scene;
	std::vector<entt::entity> m_children;

public:
	std::string Name;
	[[nodiscard]] Entity GetParent() const { return { m_parent, m_scene }; }

	CommonEntityData(const Entity& entity, const std::string& name, const Entity& parent) : m_parent(parent.m_handle), m_scene(entity.m_scene), Name(name)
	{
		if (parent) parent.GetComponent<CommonEntityData>().m_children.push_back(entity);
	};
};

template <>
struct fmt::formatter<Entity> {
	// Presentation format: 'e' - Entity name only, 'i' - Id only, 'E' - Entity name with id.
	char presentation = 'E';

	// Parses format specifications of the form ['e' | 'i' | 'E'].
	constexpr auto parse(format_parse_context& ctx)
	{
		auto it = ctx.begin(), end = ctx.end();
		if (it != end && (*it == 'e' || *it == 'i' || *it == 'E')) presentation = *it++;
		
		if (it != end && *it != '}')
			throw format_error("invalid format");
		
		return it;
	}
	
	template <typename FormatContext> auto format(const Entity& e, FormatContext& ctx)
	{
		switch (presentation)
		{
			case 'e':
				return format_to(ctx.out(), "[ent: {}]", e ? e.GetComponent<CommonEntityData>().Name : "null");
			case 'i':
				return format_to(ctx.out(), "[ent] ({:04x})", e ? static_cast<entt::entity>(e) : static_cast<entt::entity>(-1));
			default:
				return e ? format_to(ctx.out(), "[ent:{}] ({:04x})", e.GetComponent<CommonEntityData>().Name, static_cast<entt::entity>(e)) : format_to(ctx.out(), "[ent:null]");
		}
	}
};