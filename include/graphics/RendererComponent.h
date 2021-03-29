#pragma once
#include "core/Common.h"
#include "Mesh.h"
#include "Material.h"

struct RendererComponent
{
	Shared<Mesh> mesh;
	Shared<Material> material;
};