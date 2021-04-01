#pragma once
#include "CameraComponent.h"
#include "RendererComponent.h"
#include "core/Common.h"
#include "diligent/DiligentInit.h"
#include "core/Scene.h"

struct DrawSettings
{
	Shared<Material> overrideMaterial;
};

struct CullingResults
{
	std::vector<RendererComponent&> visibleRenderers;
};

class PipelineContext
{
	Shared<DiligentContext> m_ctx;

public:
	void Cull(const Shared<Scene>& scene, const CameraComponent& camera, CullingResults& outResult);
	void Draw(const CullingResults& culled, const DrawSettings& settings);
};