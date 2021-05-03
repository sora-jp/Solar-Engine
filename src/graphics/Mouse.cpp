#include "pch.h"
#include "Mouse.h"
#include "GLFW/glfw3.h"

void Mouse::Update()
{
	double tmpx, tmpy;
	glfwGetCursorPos(m_window, &tmpx, &tmpy);

	const auto lastPos = m_pos;
	m_pos.x = tmpx;
	m_pos.y = tmpy;

	m_delta = m_pos - lastPos;

	for (int i = 0; i < 5; i++)
	{
		m_buttonStates[i] = (m_buttonStates[i] << 1 | glfwGetMouseButton(m_window, i)) & 0b11;
	}
}

glm::vec2 Mouse::GetPosition() const
{
	return m_pos;
}

glm::vec2 Mouse::GetDelta() const
{
	return m_delta;
}

bool Mouse::ButtonDown(MouseButton btn) const
{
	return m_buttonStates[static_cast<int>(btn)] == 0b01;
}

bool Mouse::ButtonHeld(MouseButton btn) const
{
	return m_buttonStates[static_cast<int>(btn)] == 0b11;
}

bool Mouse::ButtonUp(MouseButton btn) const
{
	return m_buttonStates[static_cast<int>(btn)] == 0b10;
}

void Mouse::SetCursorEnabled(bool enabled) const
{
	glfwSetInputMode(m_window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
