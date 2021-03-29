#pragma once

#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Shader.h"
#include "diligent/Graphics/GraphicsEngine/interface/ShaderBindingTable.h"
#include <vector>

using namespace Diligent;

class ShaderPass
{
	friend class ShaderCompiler;
	friend class Material;
	friend class DiligentContext;

	RefCntAutoPtr<IPipelineState> m_pipelineState;
};

class Shader
{
	friend class ShaderCompiler;
	friend class Material;
	friend class DiligentContext;

	std::vector<Unique<ShaderPass>> m_passes;
};

class ShaderCompiler
{
public:
	static Shared<Shader> Compile(std::string name, std::string vs, std::string fs);
};