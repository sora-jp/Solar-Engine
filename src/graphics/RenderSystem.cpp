#include "pch.h"
#include "RenderSystem.h"
#include "GraphicsSubsystem.h"
#include "core/Entity.h"

void RenderSystem::Execute(Entity e, CameraComponent& camera, TransformComponent& transform)
{
	for (const auto& loadedScene : Scene::GetLoadedScenes())
	{
		m_pipeline->RenderCamera(loadedScene, camera, transform, m_pipelineCtx, m_target);
	}
}
