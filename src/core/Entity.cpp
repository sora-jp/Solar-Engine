#include "pch.h"
#include "Entity.h"

Entity::Entity(const entt::entity handle, const Shared<Scene>& scene) : m_handle(handle), m_scene(scene) {}

const Entity&& Entity::null = {entt::null, nullptr};
std::map<entt::id_type, std::function<void*(Entity*)>> Entity::_getters;
std::map<entt::id_type, ComponentWrapperBase*> Entity::_impls;

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