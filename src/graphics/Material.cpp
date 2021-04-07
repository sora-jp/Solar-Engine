#include "pch.h"
#include "Material.h"

#include "MaterialPropertyBlock.h"

Material::Material(const Shared<Shader> shader, bool autoInit) : shader(shader)
{
	m_mpb = MaterialPropertyBlock::Create(shader);
}