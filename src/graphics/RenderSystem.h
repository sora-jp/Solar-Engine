#pragma once

#include "core/Entity.h"
#include "core/System.h"
#include "core/TransformComponent.h"
#include "RendererComponent.h"

class RenderSystem final : public System<RendererComponent, TransformComponent>
{
public:
	void Execute(Entity e, RendererComponent& r, TransformComponent& t) override;
};
