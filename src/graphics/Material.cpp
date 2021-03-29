#include "pch.h"
#include "Material.h"

Material::Material(const Shared<Shader> shader) : m_bindings(new RefCntAutoPtr<IShaderResourceBinding>[shader->m_passes.size()]), shader(shader)
{
	for (auto i = 0; i < shader->m_passes.size(); i++)
		shader->m_passes[i]->m_pipelineState->CreateShaderResourceBinding(&m_bindings[i], true);
}

Material::~Material()
{
	delete[] m_bindings;
}