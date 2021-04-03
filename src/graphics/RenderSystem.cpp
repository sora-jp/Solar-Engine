#include "pch.h"
#include "RenderSystem.h"
#include "GraphicsSubsystem.h"
#include "RendererComponent.h"
#include "core/Entity.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "core/Log.h"

void RenderSystem::Execute(Entity e, CameraComponent& camera, TransformComponent& transform)
{
	// SOLAR_CORE_INFO("Camera FOV: {}, Clipping: {} -> {}", camera.fov, camera.nearClip, camera.farClip);
	// SOLAR_CORE_INFO("Camera Pos: {} {} {}", transform.position.x, transform.position.y, transform.position.z);
	/*const auto proj = glm::perspectiveLH_ZO(glm::radians(camera.fov), 1280.0f / 720.0f, camera.nearClip, camera.farClip);
	const auto view = glm::inverse(transform.GetTransformMatrix());
	*/
	for (const auto& loadedScene : Scene::GetLoadedScenes())
	{
		m_pipeline->RenderCamera(loadedScene, camera, transform, m_pipelineCtx, *m_target);
		//loadedScene->IterateEntities<RendererComponent, TransformComponent>([=](Entity ee, RendererComponent& r, TransformComponent& t) -> void
		//{
		//	{
		//		auto map = GraphicsSubsystem::GetCurrentContext()->MapConstants();
		//		map->model = glm::transpose(t.GetTransformMatrix());
		//		map->viewProj = glm::transpose(proj * view);
		//	}
		//	
		//	//GraphicsSubsystem::GetCurrentContext()->SetModelMatrix(t.GetTransformMatrix());
		//	GraphicsSubsystem::GetCurrentContext()->BindMaterial(r.material);
		//	GraphicsSubsystem::GetCurrentContext()->SubmitMesh(r.mesh);
		//});
	}
}
