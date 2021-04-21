#pragma once
#include "diligent/DiligentInit.h"
#include "CameraComponent.h"
#include "PipelineContext.h"
#include "core/TransformComponent.h"
#include "core/Scene.h"

class RenderPipeline
{
public:
	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	virtual void Init(const PipelineContext& ctx) = 0;
	virtual void RenderCamera(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, const PipelineContext& ctx, RenderTarget* target) = 0;
};
