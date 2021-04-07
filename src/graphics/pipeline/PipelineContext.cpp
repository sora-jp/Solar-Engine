#include "pch.h"
#include "PipelineContext.h"

#include "core/Entity.h"

void PipelineContext::Cull(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, CullingResults& outResult) const
{
	const auto vpMat = camera.GetCameraMatrix() * glm::inverse(cameraTransform.GetTransformMatrix());
	const auto rendererCount = scene->CountEntities<RendererComponent>();
	if (outResult.visibleRenderers.capacity() < rendererCount) outResult.visibleRenderers.resize(rendererCount);

	const auto frustum = Frustum::FromVPMatrix(vpMat);

	auto i = 0;
	scene->IterateEntities<const TransformComponent, const RendererComponent>([&](Entity, const TransformComponent& t, const RendererComponent& r)
	{
		const auto mat = t.GetTransformMatrix();
		const auto bounds = r.mesh->bounds.Transform(mat);

		//if (frustum.TestAABB(bounds))
		{
			outResult.visibleRenderers[i].renderer = &r;
			outResult.visibleRenderers[i].modelMatrix = glm::transpose(t.GetTransformMatrix());
			outResult.visibleRenderers[i].position = t.position;
			i++;
		}
	});

	outResult.vpMatrix = glm::transpose(vpMat);
	outResult.rendererCount = i;
}

void PipelineContext::SetupCameraProps(const CullingResults& culled) const
{
	m_ctx->GetConstants()->viewProj = culled.vpMatrix;
}

void PipelineContext::SetupCameraProps(const glm::mat4& vpMatrix) const
{
	m_ctx->GetConstants()->viewProj = glm::transpose(vpMatrix);
}

void PipelineContext::Draw(const CullingResults& culled, const DrawSettings& settings) const
{
	auto* consts = m_ctx->GetConstants();

	if (settings.overrideMaterial)
	{
		m_ctx->BindMaterial(settings.overrideMaterial);
	}
	
	for (auto i = 0; i < culled.rendererCount; i++)
	{
		const auto& r = culled.visibleRenderers[i];
		consts->model = r.modelMatrix;

		m_ctx->FlushConstants();
		
		for (auto s = 0; s < r.renderer->mesh->GetSubMeshCount(); s++) 
		{
			if (!settings.overrideMaterial) m_ctx->BindMaterial(r.renderer->GetMaterial(s));
			m_ctx->SubmitMesh(r.renderer->mesh, s);
		}
	}
}

void PipelineContext::BlitFullscreenQuad(ITexture* src, ITexture* dest, const Shared<Material>& mat) const
{
	auto* view = dest->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
	
	m_ctx->GetContext()->SetRenderTargets(1, &view, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	mat->GetProperties().Set("_MainTex", src->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

	m_ctx->BindMaterial(mat);

	const DrawAttribs attribs {3, DRAW_FLAG_VERIFY_ALL };
	
	m_ctx->GetContext()->Draw(attribs);
}

void PipelineContext::RenderFullscreenQuad(const Shared<Material>& mat) const
{
	m_ctx->BindMaterial(mat);

	const DrawAttribs attribs{ 3, DRAW_FLAG_VERIFY_ALL };
	m_ctx->GetContext()->Draw(attribs);
}