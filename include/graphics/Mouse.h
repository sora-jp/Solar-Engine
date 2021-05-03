#pragma once
#include "core/Common.h"
#include "core/Input.h"
#include "glm/glm.hpp"

enum class MouseButton
{
	Left, Right, Middle, Button4, Button5
};

class Mouse final : public InputDevice
{
	struct GLFWwindow* m_window;
	glm::vec2 m_pos;
	glm::vec2 m_delta;
	uint8_t m_buttonStates[5];

protected:
	void Update() override;

public:
	explicit Mouse(GLFWwindow* window) : InputDevice(), m_window(window), m_pos(0), m_delta(0), m_buttonStates{0, 0, 0, 0, 0} {}
	
	[[nodiscard]] glm::vec2 GetPosition() const;
	[[nodiscard]] glm::vec2 GetDelta() const;
	
	[[nodiscard]] bool ButtonDown(MouseButton btn) const;
	[[nodiscard]] bool ButtonHeld(MouseButton btn) const;
	[[nodiscard]] bool ButtonUp(MouseButton btn) const;

	void SetCursorEnabled(bool enabled) const;
};