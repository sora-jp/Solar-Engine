#pragma once
#include <random>

#include "PostProcessStack.h"

struct GTAOSettings
{
	
};

class GTAOEffect : public PostProcessEffect<GTAOSettings, EffectLocation::BeforeComposite>
{
	std::default_random_engine rand;
	std::uniform_real<float> dpi;
	std::uniform_real<float> d01;

	Shared<RenderTexture> m_gtaoRt;
public:
	[[nodiscard]] RenderTextureAttachment* GTAOResultTexture() const { return m_gtaoRt->Color(0); }
	
	void Render(RenderTarget* target, PostProcessContext& context, GTAOSettings settings) override
	{
		static glm::mat4x4 lastVP;
		static const auto m_gtao = Material::Create(ShaderCompiler::Compile("GTAO.hlsl", "vert", "frag"));
		static const auto m_blit = Material::Create(ShaderCompiler::Compile("Blit.hlsl", "vert", "frag"));

		static const auto m_gtaoFilter = ShaderCompiler::CompileCompute("GTAOFilter.hlsl", "compute");
		static const auto m_gtaoFProps = MaterialPropertyBlock::Create(m_gtaoFilter);
		
		if (m_gtaoRt == nullptr || (m_gtaoRt->Width() != target->Width() || m_gtaoRt->Height() != target->Height()))
		{
			const auto gtDesc = RenderTextureDesc { target->Width(), target->Height(), TextureFormat::RG32 };
			m_gtaoRt = RenderTexture::Create(gtDesc, 3, false);
		}

		m_gtao->GetProperties().SetTexture("_GBNormal", context.gBuffers->Color(2));
		m_gtao->GetProperties().SetTexture("_GBPosition", context.gBuffers->Color(3));
		
		m_gtao->GetProperties().Set("_Aspect", 1.f / context.camera->aspect);
		m_gtao->GetProperties().Set("_InvRTSize", 1.f / glm::vec2(target->Width(), target->Height()));
		m_gtao->GetProperties().Set("_AngleOffset", dpi(rand));
		m_gtao->GetProperties().Set("_SpatialOffset", d01(rand) - 0.5f);

		context.ctx->Context()->SetRenderTarget(m_gtaoRt->Color(2));
		context.ctx->Context()->Clear(nullptr, 1, 0);
		context.ctx->Context()->Blit(m_gtaoRt->Color(2), m_gtao);

		m_gtaoFProps->SetTexture("_AODepthCur", m_gtaoRt->Color(2));
		m_gtaoFProps->SetTexture("_AODepthHist", m_gtaoRt->Color(1));
		m_gtaoFProps->SetTexture("_GBPosition", context.gBuffers->Color(3));
		m_gtaoFProps->SetTexture("_AOOut", m_gtaoRt->Color(0), true);
		m_gtaoFProps->Set("_CameraVPDelta", lastVP);

		const glm::ivec3 groups((target->Width() >> 3) + 1, (target->Height() >> 3) + 1, 1);
		context.ctx->Context()->DispatchCompute(groups, m_gtaoFilter, *m_gtaoFProps);

		context.ctx->Context()->Blit(m_gtaoRt->Color(0), m_gtaoRt->Color(1), m_blit);
		lastVP = context.culled->vpMatrix;
	}
};