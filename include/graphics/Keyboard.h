#pragma once
#include "core/Input.h"
#include "Key.h"

class Keyboard final : public InputDevice
{
	struct GLFWwindow* m_window;
	std::map<Key, bool> m_lastState;
	std::map<Key, bool> m_curState;
	
protected:
	void Update() override;

public:
	explicit Keyboard(struct GLFWwindow* window) : InputDevice(), m_window(window) {}
};