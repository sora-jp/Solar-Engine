#include "pch.h"
#include "EditorWindow.h"

std::vector<Unique<EditorWindow>> EditorWindow::_windows;

EditorWindow* EditorWindow::Register(Unique<EditorWindow> wnd)
{
	auto* ptr = wnd.get();
	_windows.push_back(std::move(wnd));
	return ptr;
}

void EditorWindow::InternalDraw()
{
	if (ImGui::Begin(Title().c_str(), &m_open, ImGuiWindowFlags_NoCollapse | GetAdditionalFlags()))
	{
		Draw();
	}
}

void EditorWindow::Close(EditorWindow* wnd)
{
	_windows.erase(std::find_if(_windows.begin(), _windows.end(), [&](const Unique<EditorWindow>& w) { return w.get() == wnd; }));
}

void EditorWindow::UpdateAll()
{
	for (auto& window : _windows) window->InternalDraw();
}