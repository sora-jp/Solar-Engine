#pragma once

#include "core/Entity.h"
#include "core/System.h"
#include "core/TransformComponent.h"
#include "CameraComponent.h"
#include "pipeline/RenderPipeline.h"

class RenderSystem final : public System<CameraComponent, TransformComponent>
{
	RenderTarget* m_target = nullptr;
	PipelineContext m_pipelineCtx;
	Unique<RenderPipeline> m_pipeline;
	
public:
	void SetContext(const PipelineContext& ctx) { m_pipelineCtx = ctx; }
	void SetPipeline(Unique<RenderPipeline> pipeline) { m_pipeline = std::move(pipeline); m_pipeline->Init(m_pipelineCtx); }
	void SetTarget(RenderTarget* target) { m_target = target; }
	void Execute(Entity e, CameraComponent& r, TransformComponent& t) override;
};
