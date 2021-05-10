#pragma once
#include "core/Common.h"
#include "Mesh.h"
#include "Material.h"
#include "editor/EditorGUI.h"

struct RendererComponent
{
	Shared<Mesh> mesh;
	Shared<Material> material;
	std::vector<Shared<Material>> materials;

	[[nodiscard]] Shared<Material> GetMaterialForSubmesh(const int submesh) const
	{
		if (submesh >= mesh->GetSubMeshCount() || submesh < 0) return material;

		const auto matIdx = mesh->GetSubMesh(submesh).materialIndex;
		if (matIdx > materials.size() || matIdx < 0) return material;

		return materials[matIdx];
	}

	void OnInspectorGUI()
	{
		EditorGUI::InlineEditor(material, "Material");

		auto i = 0;
		for (auto& m : materials) 
		{
			i++;
			EditorGUI::InlineEditor(m, fmt::format("Material {}", i));
		}
	}
};