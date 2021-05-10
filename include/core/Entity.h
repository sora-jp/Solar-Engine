#pragma once
#include "Common.h"
#include "Assert.h"
#include "Scene.h"
#include <type_traits>
#include <utility>

#include "ComponentWrapper.h"
#include "entt/entt.hpp"

class SOLAR_API Entity final
{
	friend struct CommonEntityData;
	friend class InspectorWindow;

	static std::map<entt::id_type, std::function<void*(Entity*)>> _getters;
	static std::map<entt::id_type, ComponentWrapperBase*> _impls;
	
	entt::entity m_handle {entt::null};
	Shared<Scene> m_scene;

public:
	static const Entity&& null;
	constexpr Entity() noexcept : m_handle(entt::null), m_scene(nullptr) {}
	Entity(entt::entity handle, const Shared<Scene>& scene);
	
	template<class T, typename... Args, std::enable_if_t<std::is_constructible_v<T, Args...>, bool> = true>
	T& AddComponent(Args&&... args)
	{
		if (static auto isInitialized = false; !isInitialized)
		{
			//if constexpr (std::is_base_of_v<ComponentBase, T>) {
			//}
			auto hash = entt::type_id<T>().hash();
			_impls[hash] = reinterpret_cast<ComponentWrapperBase*>(new ComponentWrapper<T>());
			_getters[hash] = [](Entity* ent) { return static_cast<void*>(&ent->GetComponent<T>()); };
			
			isInitialized = true;
		}
		
		SOLAR_INFO("Constructing component {} (id {})", entt::type_name<T>::template value().data(), entt::type_seq<T>::template value());
		
		SOLAR_ASSERT(!HasComponent<T>());
		return m_scene->m_registry.emplace<T, Args...>(m_handle, std::forward<Args>(args)...);
	}

	template <class T>
	[[nodiscard]] T& GetComponent() const
	{
		SOLAR_ASSERT(HasComponent<T>());
		return m_scene->m_registry.get<T>(m_handle);
	}

	void Invoke(std::string msg)
	{
		m_scene->m_registry.visit(m_handle, [this, &msg](const entt::type_info info)
		{
			if (auto [wrapper, instance] = GetComponent(info); wrapper != nullptr) 
				wrapper->Call(instance, msg);
		});
	}
	
	std::pair<ComponentWrapperBase*, void*> GetComponent(const entt::type_info type)
	{
		const auto getter = _getters.find(type.hash());
		const auto impl = _impls.find(type.hash());
		
		if (getter == _getters.end() || impl == _impls.end())
		{
			SOLAR_WARN("Failed to find getter for {}", type.name().data());
			return { nullptr, nullptr };
		}

		return { impl->second, getter->second(this) };
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
	[[nodiscard]] Entity GetParent() const { return { m_parent, m_parent == entt::null ? nullptr : m_scene }; }
	[[nodiscard]] Entity GetChild(const int idx) const { return { m_children[idx], m_children[idx] == entt::null ? nullptr : m_scene }; }
	[[nodiscard]] size_t GetChildCount() const { return m_children.size(); }

	CommonEntityData(const Entity& entity, std::string name, const Entity& parent) : m_parent(parent.m_handle), m_scene(entity.m_scene), Name(std::move(name))
	{
		if (parent) parent.GetComponent<CommonEntityData>().m_children.push_back(entity);
	}
};