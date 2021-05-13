#include "pch.h"
#include "Shader.h"
#include "GraphicsSubsystem.h"
#include "diligent/DiligentWindow.h"
#include "diligent/Graphics/GraphicsEngine/interface/EngineFactory.h"
#include "shadertools/HLSLReflector.h"
#include <filesystem>

IShader* CompileSingle(ShaderCreateInfo& info, const std::string& entry, const SHADER_TYPE type)
{
	info.Desc.ShaderType = type;
	info.EntryPoint = entry.c_str();

	IShader* shader;
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateShader(info, &shader);
	return shader;
}

Shared<Shader> ShaderCompiler::Compile(const std::string& path, std::string vs, std::string fs, void configure(GraphicsPipelineDesc& desc))
{
	GraphicsPipelineStateCreateInfo pipelineInfo;

	pipelineInfo.PSODesc.Name = path.c_str();
	pipelineInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

	pipelineInfo.GraphicsPipeline.NumRenderTargets = 1;
	pipelineInfo.GraphicsPipeline.SmplDesc.Count = 8;
	pipelineInfo.GraphicsPipeline.RTVFormats[0] = GraphicsSubsystem::GetMainWindow()->GetSwapChain()->GetDesc().ColorBufferFormat;
	pipelineInfo.GraphicsPipeline.DSVFormat = GraphicsSubsystem::GetMainWindow()->GetSwapChain()->GetDesc().DepthBufferFormat;
	pipelineInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	
	pipelineInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
	if (configure != nullptr) configure(pipelineInfo.GraphicsPipeline);
	
	auto p0 = std::filesystem::current_path();
	auto p1 = std::filesystem::current_path() / "shaders";
	auto p2 = std::filesystem::current_path() / "shaders" / "builtin";

	auto res = p0.string().append(";").append(p1.string().append(";").append(p2.string()));
	
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	GraphicsSubsystem::GetCurrentContext()->GetFactory<IEngineFactory>()->CreateDefaultShaderSourceStreamFactory(res.c_str(), &pShaderSourceFactory);
	
	ShaderCreateInfo shaderInfo;
	shaderInfo.FilePath = path.c_str();
	shaderInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	shaderInfo.UseCombinedTextureSamplers = true;
	shaderInfo.pShaderSourceStreamFactory = pShaderSourceFactory;

	if (!vs.empty()) {
		pipelineInfo.pVS = CompileSingle(shaderInfo, vs, SHADER_TYPE_VERTEX);
	}

	if (!fs.empty()) {
		pipelineInfo.pPS = CompileSingle(shaderInfo, fs, SHADER_TYPE_PIXEL);
	}
	
	LayoutElement layout[] = {
		LayoutElement {0, 0, 3, VT_FLOAT32, false}, // Vertex position
		LayoutElement {1, 0, 3, VT_FLOAT32, true}, // Vertex normal
		LayoutElement {2, 0, 3, VT_FLOAT32, true}, // Vertex tangent
		LayoutElement {3, 0, 2, VT_FLOAT32, false}  // Texture coordinate
	};

	pipelineInfo.GraphicsPipeline.InputLayout.LayoutElements = layout;
	pipelineInfo.GraphicsPipeline.InputLayout.NumElements = _countof(layout);

	pipelineInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;

	ReflectionResult reflRes;
	if (!HLSLReflector::Reflect(path, vs, fs, "", pShaderSourceFactory, reflRes)) return nullptr; // TODO: Return error shader instead

	const auto varCount = reflRes.buffers.size() + reflRes.textures.size();
	auto* vars = new ShaderResourceVariableDesc[varCount];

	for (auto i = 0; i < varCount; i++)
	{
		auto* data = reflRes.GetData(i);

		auto varType = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
		if (reflRes.IsBuffer(i)) varType = data->name == "Constants" ? SHADER_RESOURCE_VARIABLE_TYPE_STATIC : SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
		
		vars[i] = ShaderResourceVariableDesc {static_cast<SHADER_TYPE>(data->usages), data->name.c_str(), varType};
	}
	
	pipelineInfo.PSODesc.ResourceLayout.Variables = vars;
	pipelineInfo.PSODesc.ResourceLayout.NumVariables = varCount;

	SamplerDesc defaultSampler
	{
		FILTER_TYPE_ANISOTROPIC, FILTER_TYPE_ANISOTROPIC, FILTER_TYPE_ANISOTROPIC,
		TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, 0, 4
	};

	auto* samplers = new ImmutableSamplerDesc[reflRes.textures.size()];

	for (auto i = 0ull; i < reflRes.textures.size(); i++)
	{
		auto& tex = reflRes.textures[i];
		samplers[i] = ImmutableSamplerDesc {static_cast<SHADER_TYPE>(tex.usages), tex.name.c_str(), defaultSampler};
	}
	
	// clang-format on
	pipelineInfo.PSODesc.ResourceLayout.ImmutableSamplers = samplers;
	pipelineInfo.PSODesc.ResourceLayout.NumImmutableSamplers = reflRes.textures.size();

	auto shader = MakeShared<Shader>();

	shader->m_type = SolarShaderType::Graphics;
	shader->m_reflectionInfo = reflRes;
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateGraphicsPipelineState(pipelineInfo, &shader->m_pipelineState);
	
	auto* consts = shader->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());

	consts = shader->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());

	delete[] vars;
	delete[] samplers;
	
	return shader;
}

Shared<Shader> ShaderCompiler::CompileCompute(const std::string& path, std::string compute, void configure(ComputePipelineStateCreateInfo& desc))
{
	ComputePipelineStateCreateInfo info;
	info.PSODesc.PipelineType = PIPELINE_TYPE_COMPUTE;

	if (configure) configure(info);

	auto p0 = std::filesystem::current_path();
	auto p1 = std::filesystem::current_path() / "shaders";
	auto p2 = std::filesystem::current_path() / "shaders" / "builtin";

	auto res = p0.string().append(";").append(p1.string().append(";").append(p2.string()));

	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	GraphicsSubsystem::GetCurrentContext()->GetFactory<IEngineFactory>()->CreateDefaultShaderSourceStreamFactory(res.c_str(), &pShaderSourceFactory);

	ShaderCreateInfo shaderInfo;
	shaderInfo.FilePath = path.c_str();
	shaderInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	shaderInfo.UseCombinedTextureSamplers = true;
	shaderInfo.pShaderSourceStreamFactory = pShaderSourceFactory;

	info.pCS = CompileSingle(shaderInfo, compute, SHADER_TYPE_COMPUTE);

	ReflectionResult reflRes;
	if (!HLSLReflector::Reflect(path, "", "", compute, pShaderSourceFactory, reflRes)) return nullptr; // TODO: Return error shader instead

	const auto varCount = reflRes.buffers.size() + reflRes.textures.size();
	auto* vars = new ShaderResourceVariableDesc[varCount];

	for (auto i = 0; i < varCount; i++)
	{
		auto* data = reflRes.GetData(i);

		auto varType = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
		if (reflRes.IsBuffer(i)) varType = data->name == "Constants" ? SHADER_RESOURCE_VARIABLE_TYPE_STATIC : SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

		vars[i] = ShaderResourceVariableDesc{ static_cast<SHADER_TYPE>(data->usages), data->name.c_str(), varType };
	}

	info.PSODesc.ResourceLayout.Variables = vars;
	info.PSODesc.ResourceLayout.NumVariables = varCount;

	SamplerDesc defaultSampler
	{
		FILTER_TYPE_ANISOTROPIC, FILTER_TYPE_ANISOTROPIC, FILTER_TYPE_ANISOTROPIC,
		TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, 0, 4
	};

	auto* samplers = new ImmutableSamplerDesc[reflRes.textures.size()];

	for (auto i = 0ull; i < reflRes.textures.size(); i++)
	{
		auto& tex = reflRes.textures[i];
		samplers[i] = ImmutableSamplerDesc{ static_cast<SHADER_TYPE>(tex.usages), tex.name.c_str(), defaultSampler };
	}

	// clang-format on
	info.PSODesc.ResourceLayout.ImmutableSamplers = samplers;
	info.PSODesc.ResourceLayout.NumImmutableSamplers = reflRes.textures.size();

	auto shader = MakeShared<Shader>();

	shader->m_type = SolarShaderType::Compute;
	shader->m_reflectionInfo = reflRes;
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateComputePipelineState(info, &shader->m_pipelineState);

	auto* consts = shader->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());

	consts = shader->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());

	delete[] vars;
	delete[] samplers;

	return shader;
}
