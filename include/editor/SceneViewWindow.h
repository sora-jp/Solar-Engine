#pragma once
#include "EditorWindow.h"
#include "core/TransformComponent.h"
#include "graphics/TextureBase.h"
#include "graphics/Texture.h"

struct CameraComponent;

class SceneViewWindow final : public EditorWindow
{
	CameraComponent& m_camera;
	TransformComponent& m_camTransform;
	Shared<RenderTexture> m_rt;
	
protected:
	[[nodiscard]] std::string Title() const override { return "Scene View"; }
public:
	explicit SceneViewWindow(CameraComponent& camera, TransformComponent& transform) : m_camera(camera), m_camTransform(transform) { }

	ImGuiWindowFlags GetAdditionalFlags() override { return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; }
	void Draw() override;
};
