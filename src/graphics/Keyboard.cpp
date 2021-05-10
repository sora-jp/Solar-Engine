#include "pch.h"
#include "Keyboard.h"
#include "GLFW/glfw3.h"

bool HasEntry(const std::map<Key, bool>& cur, const std::map<Key, bool>& last, const Key k)
{
	return cur.find(k) != cur.end() && last.find(k) != last.end();
}

void Keyboard::Update()
{
	std::swap(m_curState, m_lastState);
	
	for (auto i = static_cast<int>(Key::First); i <= static_cast<int>(Key::Last); i++)
	{
		const auto state = glfwGetKey(m_window, i);
		if (state != GLFW_INVALID_ENUM && state != GLFW_NOT_INITIALIZED)
		{
			m_curState[static_cast<Key>(i)] = state;
		}
	}
}

bool Keyboard::KeyDown(const Key k) const
{
	//if (ImGui::GetIO().WantCaptureKeyboard) return false;
	
	if (!HasEntry(m_curState, m_lastState, k)) return false;
	return m_curState.at(k) && !m_lastState.at(k);
}

bool Keyboard::KeyHeld(const Key k) const
{
	//if (ImGui::GetIO().WantCaptureKeyboard) return false;
	
	if (!HasEntry(m_curState, m_lastState, k)) return false;
	return m_curState.at(k);
}

bool Keyboard::KeyUp(const Key k) const
{
	//if (ImGui::GetIO().WantCaptureKeyboard) return false;
	
	if (!HasEntry(m_curState, m_lastState, k)) return false;
	return !m_curState.at(k) && m_lastState.at(k);
}
