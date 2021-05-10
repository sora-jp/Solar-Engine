#pragma once
#include "EditorWindow.h"
#include "graphics/TextureBase.h"

struct CameraComponent;

class SceneViewWindow final : public EditorWindow
{
	CameraComponent& m_camera;
	Shared<RenderTarget> m_rt;
	
protected:
	[[nodiscard]] std::string Title() const override { return "Scene View"; }
public:
	explicit SceneViewWindow(CameraComponent& camera) : m_camera(camera) { }

	ImGuiWindowFlags GetAdditionalFlags() override { return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; }
	void Draw() override;
};
