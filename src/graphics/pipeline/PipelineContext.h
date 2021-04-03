#pragma once
#include "CameraComponent.h"
#include "RendererComponent.h"
#include "core/Common.h"
#include "diligent/DiligentInit.h"
#include "core/Scene.h"
#include "core/TransformComponent.h"

struct DrawSettings
{
	Shared<Material> overrideMaterial;
};

struct DrawOperation
{
	//Shared<Mesh> mesh;
	//Shared<Material> material;
	const RendererComponent* renderer;
	glm::mat4 modelMatrix;
	glm::vec3 position;
};

struct CullingResults
{
	int rendererCount;
	glm::mat4 vpMatrix;
	std::vector<DrawOperation> visibleRenderers;
};

class PipelineContext
{
	Shared<DiligentContext> m_ctx;

public:
	PipelineContext() : m_ctx(nullptr) {}
	explicit PipelineContext(const Shared<DiligentContext>& dctx) : m_ctx(dctx) {}
	
	void Cull(const Shared<Scene>& scene, const CameraComponent& camera, const TransformComponent& cameraTransform, CullingResults& outResult) const;
	void SetupCameraProps(const CullingResults& culled) const;
	void SetupCameraProps(const glm::mat4& vpMatrix) const;
	void Draw(const CullingResults& culled, const DrawSettings& settings) const;

	Shared<DiligentContext> GetRawContext() const { return m_ctx; }
};