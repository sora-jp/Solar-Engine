#include "pch.h"
#include "SceneGraphWindow.h"

#include "EditorContext.h"
#include "core/Scene.h"
#include "core/Entity.h"
#include "imgui.h"

static intptr_t s_idx = 0;

void DrawEntity(const Entity&& e, const CommonEntityData& entityData)
{
	const int numChildren = entityData.GetChildCount();
	s_idx++;

	auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
	if (numChildren == 0) flags |= ImGuiTreeNodeFlags_Leaf;
	if (EditorContext::selectedEntity == e) flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(reinterpret_cast<void*>(s_idx), flags, "%s", entityData.Name.c_str()))
	{
		if (ImGui::IsItemClicked())
		{
			EditorContext::selectedEntity = e;
		}
		
		for (auto i = 0; i < numChildren; i++)
		{
			auto child = entityData.GetChild(i);
			DrawEntity(std::forward<Entity>(child), child.GetComponent<CommonEntityData>());
		}
		
		ImGui::TreePop();
	}
}

std::string SceneGraphWindow::Title() const
{
	return "Scene Graph";
}

void SceneGraphWindow::Draw()
{
	const auto& scenes = Scene::GetLoadedScenes();

	s_idx = 0;
	for (const auto& s : scenes)
	{
		s_idx++;
		if (ImGui::TreeNodeEx(reinterpret_cast<void*>(s_idx), ImGuiTreeNodeFlags_DefaultOpen, "Scene graph %lld", s_idx))
		{
			s->IterateTopLevelEntities(DrawEntity);

			ImGui::TreePop();
		}
	}
}
