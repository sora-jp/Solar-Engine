#pragma once
#include "pipeline/RenderPipeline.h"
#include <random>

#include "postprocessing/GTAOEffect.h"
#include "postprocessing/PostProcessStack.h"

enum class GBufferType
{
	Diffuse, Normal, Position, SpecMetal, Depth
};

class SimplePipeline final : public RenderPipeline
{
	Shared<Material> m_shadowmapMat, m_compositeMat;
	Shared<RenderTexture> m_rt, m_shadowmap;
	Shared<Material> m_satConv, m_satH, m_satV;
	PostProcessStack m_ppstack;
	PostProcessContext m_ppctx;
	
public:
	void Init(const PipelineContext& ctx) override;
	void RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx) const;
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

	m_shadowmap = RenderTexture::Create({1024, 1024, TextureFormat::RG32, TextureFormat::D32}, 2);
	
	m_compositeMat->GetProperties().SetTexture("_ShadowMap", m_shadowmap->Color(0));

	m_compositeMat->GetProperties().Set("_MainLight.color", glm::vec3(0));
	m_compositeMat->GetProperties().Set("_MainLight.direction", glm::normalize(glm::vec3(-1, 1, -1)));
	
	m_ppstack.Use<GTAOEffect>();
}

inline void SimplePipeline::RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx) const
{
	ctx.Context()->SetRenderTarget(target.get());
	ctx.Context()->Clear(nullptr, 1, 0);
	
	ctx.SetupCameraProps(viewToLight);
	ctx.Draw(culled, { m_shadowmapMat });

	ctx.Context()->Blit(target->Depth(),  target->Color(0), m_satConv);
	ctx.Context()->Blit(target->Color(0), target->Color(1), m_satV);
	ctx.Context()->Blit(target->Color(1), target->Color(0), m_satH);
}

/*
PerMaterial
	int _DepthMip;

	float4x4 _CameraVPDelta;
	float4x4 _CameraRays;
End
 */

static glm::mat4x4 lastVP;
inline void SimplePipeline::RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTarget* target)
{
	if (m_rt == nullptr || (m_rt->Width() != target->Width() || m_rt->Height() != target->Height()))
	{
		m_rt = RenderTexture::Create({target->Width(), target->Height(), TextureFormat::RGBA32, TextureFormat::D32}, 4);
		m_compositeMat->GetProperties().SetTexture("_GBDiffuseRough", m_rt->Color(0));
		m_compositeMat->GetProperties().SetTexture("_GBEmissionMetal", m_rt->Color(1));
		m_compositeMat->GetProperties().SetTexture("_GBNormal", m_rt->Color(2));
		m_compositeMat->GetProperties().SetTexture("_GBPosition", m_rt->Color(3));
		m_compositeMat->GetProperties().SetTexture("_GBDepth", m_rt->Depth());
	}

	
	CullingResults culled;
	ctx.Context()->SetRenderTarget(m_rt.get());
	ctx.Context()->Clear(nullptr, 1, 0);
	
	ctx.Cull(scene, camera, cameraTransform, culled);
	ctx.SetupCameraProps(culled);

	ctx.Draw(culled, { nullptr });

	m_ppctx.ctx = &ctx;
	m_ppctx.camera = &camera;
	m_ppctx.culled = &culled;
	m_ppctx.gBuffers = m_rt.get();
	
	m_ppstack.Render<EffectLocation::BeforeComposite>(target, m_ppctx);

	static const auto SIZE = 15.f;
	const auto shadowmatrix = 
		glm::orthoLH_ZO(-SIZE, SIZE, -SIZE, SIZE, 0.f, 50.f) * 
		(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0, 0, 25)) * 
			static_cast<glm::mat4>(glm::inverse(glm::quat(glm::vec3(glm::radians(45.f), glm::radians(45.f), 0.f)))));
	
	RenderLight(culled, shadowmatrix, m_shadowmap, ctx);
	
	ctx.Context()->SetRenderTarget(target);
	ctx.Context()->Clear(nullptr, 1, 0);
	
	m_compositeMat->GetProperties().Set("_MainLight.worldToLightSpace", glm::transpose(shadowmatrix));

	ctx.Context()->GetConstants()->viewProj = culled.invVpMatrix;
	ctx.Context()->GetConstants()->model = glm::transpose(shadowmatrix);
	ctx.Context()->GetConstants()->worldSpaceCamPos = cameraTransform.position;
	ctx.Context()->FlushConstants();

	m_compositeMat->GetProperties().SetTexture("_GBAmbientOcclusion", m_ppstack.Get<GTAOEffect>().GTAOResultTexture());
	m_compositeMat->GetProperties().SetTexture("_Skybox", camera.skybox.get());
	m_compositeMat->GetProperties().SetTexture("_IBL", camera.indirectIBL.get());
	
	ctx.Context()->Blit(target, m_compositeMat);

	lastVP = culled.vpMatrix;
}
