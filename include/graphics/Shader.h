#pragma once

#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Shader.h"
#include "diligent/Graphics/GraphicsEngine/interface/ShaderBindingTable.h"
#include "shadertools/HLSLReflectionTypes.h"

enum class SolarShaderType
{
	Invalid, Graphics, Compute
};

class Shader
{
	friend class ShaderCompiler;
	friend class Material;
	friend class MaterialPropertyBlock;
	friend class DiligentContext;

	SolarShaderType m_type;
	ReflectionResult m_reflectionInfo;
	Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pipelineState;

public:
	SolarShaderType Type() const { return m_type; }
};

class ShaderCompiler
{
public:
	static Shared<Shader> Compile(const std::string& path, std::string vs, std::string fs, void configure(Diligent::GraphicsPipelineDesc& desc) = nullptr);
	static Shared<Shader> CompileCompute(const std::string& path, std::string compute, void configure(Diligent::ComputePipelineStateCreateInfo& desc) = nullptr);
};