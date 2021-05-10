#pragma once

#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Shader.h"
#include "diligent/Graphics/GraphicsEngine/interface/ShaderBindingTable.h"
#include "shadertools/HLSLReflectionTypes.h"

using namespace Diligent;

class Shader
{
	friend class ShaderCompiler;
	friend class Material;
	friend class MaterialPropertyBlock;
	friend class DiligentContext;

	ReflectionResult m_reflectionInfo;
	RefCntAutoPtr<IPipelineState> m_pipelineState;
};

class ShaderCompiler
{
public:
	static Shared<Shader> Compile(const std::string& path, std::string vs, std::string fs, void configure(GraphicsPipelineDesc& desc) = nullptr);
};