#include "pch.h"
#include "Shader.h"
#include "GraphicsSubsystem.h"
#include "diligent/DiligentWindow.h"
#include "diligent/Graphics/GraphicsEngine/interface/EngineFactory.h"

IShader* CompileSingle(ShaderCreateInfo info, std::string source, std::string entry, SHADER_TYPE type)
{
	info.Desc.ShaderType = type;
	info.Source = source.c_str();
	info.EntryPoint = entry.c_str();

	IShader* shader;
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateShader(info, &shader);
	return shader;
}

Shared<Shader> ShaderCompiler::Compile(std::string name, std::string src, std::string vs, std::string fs, void configure(GraphicsPipelineDesc& desc))
{
	GraphicsPipelineStateCreateInfo pipelineInfo;

	pipelineInfo.PSODesc.Name = name.c_str();
	pipelineInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

	pipelineInfo.GraphicsPipeline.NumRenderTargets = 1;
	pipelineInfo.GraphicsPipeline.RTVFormats[0] = GraphicsSubsystem::GetMainWindow()->GetSwapChain()->GetDesc().ColorBufferFormat;
	pipelineInfo.GraphicsPipeline.DSVFormat = GraphicsSubsystem::GetMainWindow()->GetSwapChain()->GetDesc().DepthBufferFormat;
	pipelineInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	
	pipelineInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
	if (configure != nullptr) configure(pipelineInfo.GraphicsPipeline);

	//RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	//GraphicsSubsystem::GetCurrentContext()->GetFactory<IEngineFactory>()->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	
	ShaderCreateInfo shaderInfo;
	shaderInfo.EntryPoint = "main";
	shaderInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	shaderInfo.UseCombinedTextureSamplers = true;
	//shaderInfo.pShaderSourceStreamFactory = pShaderSourceFactory;

	if (!vs.empty()) {
		pipelineInfo.pVS = CompileSingle(shaderInfo, src, vs, SHADER_TYPE_VERTEX);
	}

	if (!fs.empty()) {
		pipelineInfo.pPS = CompileSingle(shaderInfo, src, fs, SHADER_TYPE_PIXEL);
	}

	LayoutElement layout[] = {
		LayoutElement {0, 0, 3, VT_FLOAT32, false}, // Vertex position
		LayoutElement {1, 0, 3, VT_FLOAT32, true}, // Vertex normal
		LayoutElement {2, 0, 2, VT_FLOAT32, false}  // Texture coordinate
	};

	pipelineInfo.GraphicsPipeline.InputLayout.LayoutElements = layout;
	pipelineInfo.GraphicsPipeline.InputLayout.NumElements = _countof(layout);

	pipelineInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

	//if (!fs.empty()) {
		// clang-format off
		// Shader variables should typically be mutable, which means they are expected
		// to change on a per-instance basis
		ShaderResourceVariableDesc Vars[] =
		{
			{SHADER_TYPE_PIXEL, "_MainTex", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_GBufferDiffuse", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_GBufferPosition", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_GBufferNormal", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_GBufferSpecMetal", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_GBufferDepth", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			{SHADER_TYPE_PIXEL, "_ShadowMap", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
		};
		// clang-format on
		pipelineInfo.PSODesc.ResourceLayout.Variables = Vars;
		pipelineInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

		// clang-format off
		// Define immutable sampler for _MainTex. Immutable samplers should be used whenever possible
		SamplerDesc SamLinearClampDesc
		{
			FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
			TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
		};
		ImmutableSamplerDesc ImtblSamplers[] =
		{
			{SHADER_TYPE_PIXEL, "_MainTex", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_GBufferDiffuse", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_GBufferPosition", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_GBufferNormal", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_GBufferSpecMetal", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_GBufferDepth", SamLinearClampDesc},
			{SHADER_TYPE_PIXEL, "_ShadowMap", SamLinearClampDesc}
		};
		// clang-format on
		pipelineInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
		pipelineInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
	//}

	auto shader = MakeShared<Shader>();
	auto pass = MakeUnique<ShaderPass>();
	
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateGraphicsPipelineState(pipelineInfo, &pass->m_pipelineState);
	
	auto* consts = pass->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());

	consts = pass->m_pipelineState->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Constants");
	if (consts) consts->Set(GraphicsSubsystem::GetCurrentContext()->GetConstantBuffer());
	
	shader->m_passes.push_back(std::move(pass));
	return shader;
}