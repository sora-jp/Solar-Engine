#pragma once
#include "pipeline/RenderPipeline.h"
#include <random>

enum class GBufferType
{
	Diffuse, Normal, Position, SpecMetal, Depth
};

class SimplePipeline final : public RenderPipeline
{
	std::default_random_engine rand;
	std::uniform_real<float> dpi;
	std::uniform_real<float> d01;
	
	Shared<Material> m_shadowmapMat, m_compositeMat;
	Shared<RenderTarget> m_rt, m_shadowmap;
	Shared<Material> m_satConv, m_satH, m_satV;
	Shared<Material> m_gtao, m_blit;

	Shared<RenderTarget> m_gtaoLast, m_gtaoCur, m_gtaoTemp;

	Shared<Shader> m_gtaoFilter;
	Unique<MaterialPropertyBlock> m_gtaoFProps;
	
public:
	void Init(const PipelineContext& ctx) override;
	void RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTarget>& target, const PipelineContext& ctx) const;
	void RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTarget* target) override;
};

inline void SimplePipeline::Init(const PipelineContext& ctx)
{
	std::random_device dev;
	rand = std::default_random_engine(dev());
	dpi = std::uniform_real(0.f, 6.28f);
	d01 = std::uniform_real(0.f, 1.f);
	
	m_shadowmapMat = Material::Create(ShaderCompiler::Compile("ShadowMapShader.hlsl", "vert", "", [](GraphicsPipelineDesc& desc)
	{
		desc.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
		desc.NumRenderTargets = 0;
		desc.RasterizerDesc.CullMode = CULL_MODE_NONE;
	}));

	m_gtao = Material::Create(ShaderCompiler::Compile("GTAO.hlsl", "vert", "frag"));
	m_blit = Material::Create(ShaderCompiler::Compile("Blit.hlsl", "vert", "frag"));

	m_gtaoFilter = ShaderCompiler::CompileCompute("GTAOFilter.hlsl", "compute");
	m_gtaoFProps = MaterialPropertyBlock::Create(m_gtaoFilter);
	
	m_compositeMat = Material::Create(ShaderCompiler::Compile("GBufferComposite.hlsl", "vert", "frag"));

	m_satConv = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragConv"));
	m_satH = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragH"));
	m_satV = Material::Create(ShaderCompiler::Compile("ShadowProcessSAT.hlsl", "vert", "fragV"));

	m_shadowmap = RenderTarget::Create(2, {TEX_FORMAT_RG32_FLOAT, 1024, 1024}, {TEX_FORMAT_D32_FLOAT, 1024, 1024});
	
	m_compositeMat->GetProperties().SetTexture("_ShadowMap", m_shadowmap->Color(0));

	m_compositeMat->GetProperties().Set("_MainLight.color", glm::vec3(0));
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
		m_rt = RenderTarget::Create(4, { TEX_FORMAT_RGBA32_FLOAT, target->Width(), target->Height() }, { TEX_FORMAT_D32_FLOAT, target->Width(), target->Height() });
		m_compositeMat->GetProperties().SetTexture("_GBDiffuseRough", m_rt->Color(0));
		m_compositeMat->GetProperties().SetTexture("_GBEmissionMetal", m_rt->Color(1));
		m_compositeMat->GetProperties().SetTexture("_GBNormal", m_rt->Color(2));
		m_compositeMat->GetProperties().SetTexture("_GBPosition", m_rt->Color(3));
		m_compositeMat->GetProperties().SetTexture("_GBDepth", m_rt->Depth());

		m_gtao->GetProperties().SetTexture("_GBNormal", m_rt->Color(2));
		m_gtao->GetProperties().SetTexture("_GBPosition", m_rt->Color(3));

		const auto gtDesc = RenderTargetDescription{ TEX_FORMAT_RG32_FLOAT, target->Width(), target->Height() };
		m_gtaoCur  = RenderTarget::Create(1, gtDesc, nullptr);
		m_gtaoLast = RenderTarget::Create(1, gtDesc, nullptr);
		m_gtaoTemp = RenderTarget::Create(1, gtDesc, nullptr);
	}
	
	CullingResults culled;
	ctx.GetRawContext()->SetRenderTarget(m_rt.get());
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.Cull(scene, camera, cameraTransform, culled);
	ctx.SetupCameraProps(culled);

	ctx.Draw(culled, { nullptr });
	
	m_gtao->GetProperties().Set("_Aspect", 1.f / camera.aspect);
	m_gtao->GetProperties().Set("_InvRTSize", 1.f / glm::vec2(target->Width(), target->Height()));
	m_gtao->GetProperties().Set("_AngleOffset", dpi(rand));
	m_gtao->GetProperties().Set("_SpatialOffset", d01(rand) - 0.5f);

	ctx.GetRawContext()->SetRenderTarget(m_gtaoTemp.get());
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	ctx.RenderFullscreenQuad(m_gtao);

	m_gtaoFProps->SetTexture("_AODepthCur", m_gtaoTemp->Color(0));
	m_gtaoFProps->SetTexture("_AODepthHist", m_gtaoLast->Color(0));
	m_gtaoFProps->SetTexture("_GBPosition", m_rt->Color(3));
	m_gtaoFProps->SetTexture("_AOOut", m_gtaoCur->Color(0), true);
	m_gtaoFProps->Set("_CameraVPDelta", lastVP);

	const glm::ivec3 groups((target->Width() >> 3) + 1, (target->Height() >> 3) + 1, 1);
	ctx.GetRawContext()->DispatchCompute(groups, m_gtaoFilter, *m_gtaoFProps);
	
	ctx.BlitFullscreenQuad(m_gtaoCur->Color(0), m_gtaoLast->Color(0), m_blit);
	
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

	m_compositeMat->GetProperties().SetTexture("_GBAmbientOcclusion", m_gtaoCur->Color(0));
	m_compositeMat->GetProperties().SetTexture("_Skybox", *camera.skybox);
	m_compositeMat->GetProperties().SetTexture("_IBL", *camera.indirectIBL);
	
	ctx.RenderFullscreenQuad(m_compositeMat);

	lastVP = culled.vpMatrix;
}
