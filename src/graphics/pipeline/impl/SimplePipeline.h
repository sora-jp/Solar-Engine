#pragma once
#include "pipeline/RenderPipeline.h"

enum class GBufferType
{
	Diffuse, Normal, Position, SpecMetal, Depth
};

class SimplePipeline final : public RenderPipeline
{
	Shared<Material> m_shadowmapMat, m_compositeMat;
	Shared<RenderTarget> m_rt, m_shadowmap;
	Shared<Material> m_satConv, m_satH, m_satV;
	
public:
	void Init(const PipelineContext& ctx) override;
	void RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTarget>& target, const PipelineContext& ctx) const;
	void RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTarget* target) override;
};

inline void SimplePipeline::Init(const PipelineContext& ctx)
{
	m_shadowmapMat = Material::Create(ShaderCompiler::Compile("ShadowMapShader.hlsl", "vert", "", [](GraphicsPipelineDesc& desc)
	{
		desc.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
		desc.NumRenderTargets = 0;
		desc.RasterizerDesc.CullMode = CULL_MODE_NONE;
	}));
	
	m_compositeMat = Material::Create(ShaderCompiler::Compile("GBufferComposite.hlsl", "vert", "frag"));

	m_satConv = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragConv"));
	m_satH = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragH"));
	m_satV = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragV"));

	m_shadowmap = RenderTarget::Create(2, {TEX_FORMAT_RG32_FLOAT, 1024, 1024}, {TEX_FORMAT_D16_UNORM, 1024, 1024});
	m_rt = RenderTarget::Create(4, {TEX_FORMAT_RGBA32_FLOAT, 1920, 1080}, {TEX_FORMAT_D16_UNORM, 1920, 1080});
	
	m_compositeMat->GetProperties().SetTexture("_GBDiffuseRough",  m_rt->Color(0));
	m_compositeMat->GetProperties().SetTexture("_GBEmissionMetal", m_rt->Color(1));
	m_compositeMat->GetProperties().SetTexture("_GBNormal",        m_rt->Color(2));
	m_compositeMat->GetProperties().SetTexture("_GBPosition",      m_rt->Color(3));
	m_compositeMat->GetProperties().SetTexture("_GBDepth",         m_rt->Depth());
	
	m_compositeMat->GetProperties().SetTexture("_ShadowMap", m_shadowmap->Color(0));

	m_compositeMat->GetProperties().Set("_MainLight.color", glm::vec3(.5, .5, .5));
	m_compositeMat->GetProperties().Set("_MainLight.direction", glm::normalize(glm::vec3(-1, 1, -1)));
}

inline void SimplePipeline::RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTarget>& target, const PipelineContext& ctx) const
{
	ctx.GetRawContext()->SetRenderTarget(target.get());
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.SetupCameraProps(viewToLight);
	ctx.Draw(culled, { m_shadowmapMat });

	ctx.BlitFullscreenQuad(target->Depth(),  target->Color(0), m_satConv);
	ctx.BlitFullscreenQuad(target->Color(0), target->Color(1), m_satV);
	ctx.BlitFullscreenQuad(target->Color(1), target->Color(0), m_satH);
}

inline void SimplePipeline::RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTarget* target)
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
	
	RenderLight(culled, shadowmatrix, m_shadowmap, ctx);
	
	ctx.GetRawContext()->SetRenderTarget(target);
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	m_compositeMat->GetProperties().Set("_MainLight.worldToLightSpace", glm::transpose(shadowmatrix));

	ctx.GetRawContext()->GetConstants()->viewProj = culled.invVpMatrix;
	ctx.GetRawContext()->GetConstants()->model = glm::transpose(shadowmatrix);
	ctx.GetRawContext()->GetConstants()->worldSpaceCamPos = cameraTransform.position;
	ctx.GetRawContext()->FlushConstants();

	m_compositeMat->GetProperties().SetTexture("_Skybox", *camera.skybox);
	ctx.RenderFullscreenQuad(m_compositeMat);
}
