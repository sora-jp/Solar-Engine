#include "pch.h"
#include "RenderSystem.h"
#include "GraphicsSubsystem.h"

void RenderSystem::Execute(Entity e, RendererComponent& renderer, TransformComponent& transform)
{
	GraphicsSubsystem::GetCurrentContext()->BindMaterial(renderer.material);
	GraphicsSubsystem::GetCurrentContext()->Submit(renderer.mesh);
}
