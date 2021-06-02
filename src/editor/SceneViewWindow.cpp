#include "pch.h"
#include "SceneViewWindow.h"

#include "EditorContext.h"
#include "graphics/CameraComponent.h"
#include "glm/glm.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "ImGuizmo.h"
#include "ImGuiDebugWindow.h"

using namespace Diligent;
void SceneViewWindow::Draw()
{
	const auto sz = ImGui::GetWindowSize();
	const glm::uvec2 szi(sz.x, sz.y);

	if (m_rt == nullptr || (m_rt->Width() != szi.x || m_rt->Height() != szi.y))
		m_rt = RenderTexture::Create({ szi.x, szi.y, TextureFormat::RGBA8, TextureFormat::D24_S8 });

	m_camera.target = m_rt;

	auto pos = ImGui::GetWindowPos();
	pos.y += 20;
	//pos.x -= ImGui::GetStyle().WindowPadding.x;
	//pos.y -= ImGui::GetStyle().WindowPadding.y;
	
	auto max = ImVec2(pos.x + sz.x, pos.y + sz.y - 20);
	
	ImGui::PushClipRect(pos, max, false);
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::GetWindowDrawList()->AddImage(m_camera.target->Color(0)->RawResourceHandle(), pos, max);

	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(pos.x, pos.y, sz.x, sz.y);
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);

	ImGuizmo::SetOrthographic(false);
	//ImGuizmo::DrawGrid(glm::value_ptr(glm::inverse(m_camTransform.GetTransformMatrix())), glm::value_ptr(m_camera.GetCameraMatrix()), glm::value_ptr(glm::identity<glm::mat4>()), 100);

	//auto mat = m_camTransform.GetTransformMatrix();
	//ImGuizmo::ViewManipulate(glm::value_ptr(mat), 5.f, pos, ImVec2(100, 100), 0x80000000);
	//mat = mat;

	//if (ImGuizmo::IsUsing()) 
	//{
		//glm::vec3 skew;
		//glm::vec4 perspective;
		//glm::quat rot;
		//glm::decompose(mat, m_camTransform.scale, rot, m_camTransform.position, skew, perspective);
		//m_camTransform.rotation = rot;
	//}

	if (EditorContext::selectedEntity != Entity::null)
	{
		auto& t = EditorContext::selectedEntity.GetComponent<TransformComponent>();
		auto tmat = t.GetTransformMatrix();

		static const auto OPS = ImGuizmo::OPERATION::TRANSLATE | ImGuizmo::OPERATION::ROTATE | ImGuizmo::OPERATION::SCALE;
		ImGuizmo::Manipulate(glm::value_ptr(glm::inverse(m_camTransform.GetTransformMatrix())), glm::value_ptr(m_camera.GetCameraMatrix()), OPS, ImGuizmo::MODE::LOCAL, glm::value_ptr(tmat));

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat rot;
		glm::decompose(tmat, t.scale, rot, t.position, skew, perspective);
		t.rotation = rot;
	}
	
	ImGui::PopClipRect();
	DrawDebugWindow();
}
