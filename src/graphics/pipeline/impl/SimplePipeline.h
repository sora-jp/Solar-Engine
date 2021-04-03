#pragma once
#include "pipeline/RenderPipeline.h"

#include "ShadowMapShader.hlsl.h"
#include "GBufferComposite.hlsl.h"

enum class GBufferType
{
	Diffuse, Normal, Position, SpecMetal, Depth
};

class SimplePipeline final : public RenderPipeline
{
	Shared<Mesh> m_quad;
	Shared<Material> m_shadowmapMat, m_compositeMat;
	RefCntAutoPtr<ITexture> m_gbuffers[5], m_shadowmap;
	ITextureView* m_gbuffersTargets[5] = {};
	ITextureView* m_gbuffersResources[5] = {};
	Shared<RenderTexture> m_rt, m_shadowmapRt;
	
public:
	void Init(const PipelineContext& ctx) override;
	void RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx) const;
	void RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTexture& target) override;
};

inline void SimplePipeline::Init(const PipelineContext& ctx)
{
	m_quad = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\plane.fbx");
	
	m_shadowmapMat = Material::Create(ShaderCompiler::Compile("Shadowmap", ShadowMapShaderHlsl, "vert", "", [](GraphicsPipelineDesc& desc)
	{
		desc.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
		desc.NumRenderTargets = 0;
	}));

	m_compositeMat = Material::Create(ShaderCompiler::Compile("Composite", GBufferCompositeHlsl, "vert", "frag"));
	
	m_gbuffers[0] = ctx.GetRawContext()->CreateTexture();
	m_gbuffers[1] = ctx.GetRawContext()->CreateTexture();
	m_gbuffers[2] = ctx.GetRawContext()->CreateTexture();
	m_gbuffers[3] = ctx.GetRawContext()->CreateTexture();
	m_gbuffers[4] = ctx.GetRawContext()->CreateDepthTexture();

	for (auto i = 0; i < 5; i++)
	{
		//m_gbuffersResources[i] = m_gbuffers[i]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
		m_gbuffersTargets[i] = m_gbuffers[i]->GetDefaultView(i == 4 ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET);
	}

	m_shadowmap = ctx.GetRawContext()->CreateDepthTexture();
	m_shadowmapRt = MakeShared<RenderTexture>(0, nullptr, m_shadowmap->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL));
	
	m_rt = MakeShared<RenderTexture>(4, m_gbuffersTargets, m_gbuffersTargets[4]);
}

inline void SimplePipeline::RenderLight(const CullingResults& culled, const glm::mat4& viewToLight, const Shared<RenderTexture>& target, const PipelineContext& ctx) const
{
	ctx.GetRawContext()->SetRenderTarget(*target);
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.SetupCameraProps(viewToLight);
	ctx.Draw(culled, { m_shadowmapMat });
}

inline void SimplePipeline::RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTexture& target)
{
	CullingResults culled;
	ctx.GetRawContext()->SetRenderTarget(*m_rt);
	ctx.GetRawContext()->Clear(nullptr, 1, 0);
	
	ctx.Cull(scene, camera, cameraTransform, culled);
	ctx.SetupCameraProps(culled);

	ctx.Draw(culled, { nullptr });

	const auto shadowmatrix = 
		glm::orthoLH_ZO(-10.f, 10.f, -10.f, 10.f, 0.f, 100.f) * 
		(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0, 0, 50)) * 
			static_cast<glm::mat4>(glm::inverse(glm::quat(glm::vec3(glm::radians(45.f), glm::radians(45.f), 0.f)))));
	
	RenderLight(culled, shadowmatrix, m_shadowmapRt, ctx);

	m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_GBufferDiffuse")->Set(m_gbuffers[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_GBufferNormal")->Set(m_gbuffers[1]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	//m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_GBufferDepth")->Set(m_gbuffers[4]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_ShadowMap")->Set(m_shadowmap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_GBufferPosition")->Set(m_gbuffers[2]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	//m_compositeMat->GetBindingsForPass(0)->GetVariableByName(SHADER_TYPE_PIXEL, "_GBufferSpecMetal")->Set(m_gbuffersResources[3]);

	ctx.GetRawContext()->SetRenderTarget(target);
	ctx.GetRawContext()->Clear(nullptr, 1, 0);

	ctx.SetupCameraProps(culled);
	ctx.GetRawContext()->GetConstants()->model = glm::transpose(shadowmatrix);
	ctx.GetRawContext()->FlushConstants();
	
	ctx.GetRawContext()->BindMaterial(m_compositeMat);
	ctx.GetRawContext()->SubmitMesh(m_quad);
}
