#pragma once
#include "core/Common.h"
#include "Shader.h"
#include "MaterialPropertyBlock.h"

class Material
{
	friend class DiligentContext;
	Unique<MaterialPropertyBlock> m_mpb;

public:
	const Shared<Shader> shader;
	[[nodiscard]] MaterialPropertyBlock& GetProperties() const { return *m_mpb; }

	static Shared<Material> Create(const Shared<Shader>& shader, const bool autoInit = true) { return Shared<Material>(new Material(shader, autoInit)); }

private:
	explicit Material(Shared<Shader> shader, bool autoInit);
};
