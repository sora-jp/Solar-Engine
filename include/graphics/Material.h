#pragma once
#include "core/Common.h"
#include "Shader.h"
#include "diligent/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"

class Material
{
	friend class DiligentContext;
	RefCntAutoPtr<IShaderResourceBinding>* const m_bindings = nullptr;

public:
	const Shared<Shader> shader;
	[[nodiscard]] IShaderResourceBinding* GetBindingsForPass(const int pass) const { return m_bindings[pass]; }

	~Material();

	static Shared<Material> Create(const Shared<Shader> shader) { return Shared<Material>(new Material(shader)); }
	
private:
	explicit Material(Shared<Shader> shader);
};
