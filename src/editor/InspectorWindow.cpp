#include "pch.h"
#include "InspectorWindow.h"
#include "EditorContext.h"
#include "core/TransformComponent.h"
#include "EditorGUI.h"
#include <regex>

std::string InspectorWindow::Title() const
{
	return "Inspector";
}

static const std::unordered_set<entt::id_type> HIDDEN_COMPONENT_TYPES = {
	entt::type_hash<CommonEntityData>::value(),
	entt::type_hash<TransformComponent>::value(),
};

void DrawDefaultComponents(Entity& e)
{
	auto& data = e.GetComponent<CommonEntityData>();
	auto& transform = e.GetComponent<TransformComponent>();
	
	EditorGUI::Field(data.Name, "Name");
	
	EditorGUI::Separator();

	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) 
	{
		EditorGUI::Field(transform.position, "Position");
		EditorGUI::Field(transform.rotation, "Rotation");
		EditorGUI::Field(transform.scale, "Scale");
		
		//auto euler = glm::eulerAngles(transform.rotation);
		//if (ImGui::InputFloat3("Rotation", glm::value_ptr(euler))) transform.rotation = glm::quat(euler);

		//ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale));
	}
}

void InspectorWindow::Draw()
{
	auto& e = EditorContext::selectedEntity;
	if (e == Entity::null) 
	{
		return;
	}

	DrawDefaultComponents(e);

	auto& r = e.m_scene->m_registry;
	r.visit(e, [&](const entt::type_info type)
	{
		if (HIDDEN_COMPONENT_TYPES.find(type.hash()) != HIDDEN_COMPONENT_TYPES.end()) return;
		const auto [impl, instance] = e.GetComponent(type);

		const std::regex extractComponentName(R"(^.*? (\w+?)(?:Component)?$)", std::regex_constants::ECMAScript | std::regex_constants::icase);
		auto sn = std::string(type.name());
		std::smatch matches;
		
		if (std::regex_search(sn, matches, extractComponentName))
		{
			sn = matches.str(1);
		}

		ImGui::Separator();
		
		if (ImGui::TreeNodeEx(type.name().data(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, "%s", sn.c_str()))
		{	
			if (instance != nullptr)
			{
				impl->Call(instance, "OnInspectorGUI");
			}

			//ImGui::TreePop();
		}
		
		//SOLAR_CORE_INFO("Drawing inspector for {}", type.name().data());
	});
}
