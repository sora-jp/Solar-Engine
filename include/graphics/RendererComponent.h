#pragma once
#include "core/Common.h"
#include "Mesh.h"
#include "Material.h"

struct RendererComponent
{
	Shared<Mesh> mesh;
	Shared<Material> material;
	std::vector<Shared<Material>> materials;

	Shared<Material> GetMaterial(const int submesh) const
	{
		if (submesh >= materials.size() || submesh < 0) return material;
		return materials[submesh];
	}
};