#pragma once

#include "core/Entity.h"
#include "core/System.h"
#include "core/TransformComponent.h"
#include "CameraComponent.h"

class RenderSystem final : public System<CameraComponent, TransformComponent>
{
public:
	void Execute(Entity e, CameraComponent& r, TransformComponent& t) override;
};
