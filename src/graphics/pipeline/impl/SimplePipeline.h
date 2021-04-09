#pragma once
#include "pipeline/RenderPipeline.h"
#include <math.h>

enum class GBufferType
{
	Diffuse, Normal, Position, SpecMetal, Depth
};

class SimplePipeline final : public RenderPipeline
{
	Shared<Material> m_shadowmapMat, m_compositeMat;
	RefCntAutoPtr<ITexture> m_gbuffers[5], m_shadowmap;
	ITextureView* m_gbuffersTargets[5] = {};
	ITextureView* m_gbuffersResources[5] = {};
	Shared<RenderTexture> m_rt, m_shadowmapRt;

	RefCntAutoPtr<ITexture> m_satA, m_satB;
	Shared<Material> m_satConv, m_satH, m_satV;
	RefCntAutoPtr<IBuffer> m_dataBuf;
	
public:
	void Init(const PipelineContext& ctx) override;
	void RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx);
	void RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTexture* target) override;
};

inline void SimplePipeline::Init(const PipelineContext& ctx)
{
	m_shadowmapMat = Material::Create(ShaderCompiler::Compile("ShadowMapShader.hlsl", "vert", "", [](GraphicsPipelineDesc& desc)
	{
		desc.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
		desc.NumRenderTargets = 0;
		desc.RasterizerDesc.CullMode = CULL_MODE_NONE;
		//desc.RasterizerDesc.SlopeScaledDepthBias = 2;
		//desc.RasterizerDesc.DepthBias = 2;
	}));
	
	m_compositeMat = Material::Create(ShaderCompiler::Compile("GBufferComposite.hlsl", "vert", "frag"));

	m_satConv = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragConv"));
	m_satH = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragH"));
	m_satV = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragV"));
	
	m_gbuffers[0] = ctx.GetRawContext()->CreateTexture(1920, 1080, TEX_FORMAT_RGBA32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	m_gbuffers[1] = ctx.GetRawContext()->CreateTexture(1920, 1080, TEX_FORMAT_RGBA32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	m_gbuffers[2] = ctx.GetRawContext()->CreateTexture(1920, 1080, TEX_FORMAT_RGBA32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	m_gbuffers[3] = ctx.GetRawContext()->CreateTexture(1920, 1080, TEX_FORMAT_RGBA32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	m_gbuffers[4] = ctx.GetRawContext()->CreateTexture(1920, 1080, TEX_FORMAT_D16_UNORM, BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE);

	for (auto i = 0; i < 5; i++)
	{
		m_gbuffersTargets[i] = m_gbuffers[i]->GetDefaultView(i == 4 ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET);
	}

	m_shadowmap = ctx.GetRawContext()->CreateTexture(1024, 1024, TEX_FORMAT_D16_UNORM, BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE);
	m_shadowmapRt = MakeShared<RenderTexture>(0, nullptr, m_shadowmap->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL));
	
	m_rt = MakeShared<RenderTexture>(4, m_gbuffersTargets, m_gbuffersTargets[4]);

	m_satA = ctx.GetRawContext()->CreateTexture(1024, 1024, TEX_FORMAT_RG32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	m_satB = ctx.GetRawContext()->CreateTexture(1024, 1024, TEX_FORMAT_RG32_FLOAT, BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);

	m_compositeMat->GetProperties().Set("_GBufferDiffuse", m_gbuffers[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetProperties().Set("_GBufferNormal", m_gbuffers[1]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetProperties().Set("_GBufferPosition", m_gbuffers[2]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetProperties().Set("_GBufferSpecMetal", m_gbuffers[3]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetProperties().Set("_GBufferDepth", m_gbuffers[4]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetProperties().Set("_ShadowMap", m_satA->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
}

inline void SimplePipeline::RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx)
{
	ctx.GetRawContext()->SetRenderTarget(target.get());
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.SetupCameraProps(viewToLight);
	ctx.Draw(culled, { m_shadowmapMat });

	ctx.BlitFullscreenQuad(m_shadowmap, m_satA, m_satConv);
	ctx.BlitFullscreenQuad(m_satA, m_satB, m_satV);
	ctx.BlitFullscreenQuad(m_satB, m_satA, m_satH);
}

inline void SimplePipeline::RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTexture* target)
{
	CullingResults culled;
	ctx.GetRawContext()->SetRenderTarget(m_rt.get());
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.Cull(scene, camera, cameraTransform, culled);
	ctx.SetupCameraProps(culled);

	ctx.Draw(culled, { nullptr });

	static const auto SIZE = 15.f;
	const auto shadowmatrix = 
		glm::orthoLH_ZO(-SIZE, SIZE, -SIZE, SIZE, 0.f, 50.f) * 
		(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0, 0, 25)) * 
			static_cast<glm::mat4>(glm::inverse(glm::quat(glm::vec3(glm::radians(45.f), glm::radians(45.f), 0.f)))));
	
	RenderLight(culled, shadowmatrix, m_shadowmapRt, ctx);

	ctx.GetRawContext()->SetRenderTarget(target);
	ctx.GetRawContext()->Clear(nullptr, 1, 0);

	ctx.GetRawContext()->GetConstants()->viewProj = culled.invVpMatrix;
	ctx.GetRawContext()->GetConstants()->model = glm::transpose(shadowmatrix);
	ctx.GetRawContext()->GetConstants()->worldSpaceCamPos = cameraTransform.position;
	ctx.GetRawContext()->FlushConstants();

	m_compositeMat->GetProperties().Set("_Skybox", camera.skybox->GetTextureView());
	//m_compositeMat->GetProperties().Set("_IBL", camera.indirectIBL->GetTextureView());
	ctx.RenderFullscreenQuad(m_compositeMat);
}
