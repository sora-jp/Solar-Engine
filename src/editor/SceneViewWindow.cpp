#include "pch.h"
#include "SceneViewWindow.h"
#include "graphics/CameraComponent.h"
#include "glm/glm.hpp"

void SceneViewWindow::Draw()
{
	const auto sz = ImGui::GetWindowSize();
	const glm::uvec2 szi(sz.x, sz.y);

	if (m_rt == nullptr || (m_rt->Width() != szi.x || m_rt->Height() != szi.y))
		m_rt = RenderTarget::Create(1, { TEX_FORMAT_RGBA8_UNORM, szi.x, szi.y }, { TEX_FORMAT_D24_UNORM_S8_UINT, szi.x, szi.y });

	m_camera.target = m_rt;

	auto pos = ImGui::GetCursorScreenPos();
	pos.x -= ImGui::GetStyle().WindowPadding.x;
	pos.y -= ImGui::GetStyle().WindowPadding.y;
	
	auto max = ImVec2(pos.x + sz.x, pos.y + sz.y);
	
	ImGui::PushClipRect(pos, max, false);
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(m_camera.target->Color(0).ToImGui(), sz);
	ImGui::PopClipRect();
}
