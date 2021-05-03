#include "pch.h"
#include "Keyboard.h"
#include "GLFW/glfw3.h"

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