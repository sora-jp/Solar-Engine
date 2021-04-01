#include "pch.h"
#include "PipelineContext.h"

#include "core/Entity.h"
#include "core/TransformComponent.h"

int PipelineContext::Cull(const Shared<Scene>& scene, const CameraComponent& camera, CullingResults& outResult)
{
	const auto projMat = camera.GetCameraMatrix();
	const auto rendererCount = scene->CountEntities<RendererComponent>();
	if (outResult.visibleRenderers.capacity() < rendererCount) outResult.visibleRenderers.resize(rendererCount);

	auto i = 0;
	scene->IterateEntities<TransformComponent, RendererComponent>([&](Entity e, TransformComponent& t, RendererComponent& r)
	{
		r.GetBounds();
	});
}

void PipelineContext::Draw(const CullingResults& culled, const DrawSettings& settings)
{
	
}
