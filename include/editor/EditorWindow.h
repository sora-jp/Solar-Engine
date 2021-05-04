#pragma once
#include "core/Common.h"
#include <vector>

class EditorWindow
{
	static std::vector<Unique<EditorWindow>> _windows;
	static EditorWindow* Register(Unique<EditorWindow> wnd);
	
	bool m_open = true;
	void InternalDraw();

protected:
	[[nodiscard]] virtual std::string Title() const = 0;

public:
	EditorWindow() = default;
	virtual ~EditorWindow() = default;

	virtual ImGuiWindowFlags GetAdditionalFlags() { return 0; }
	virtual void Draw() = 0;

	template <typename WindowType, typename ... Args, std::enable_if_t<std::is_base_of_v<EditorWindow, WindowType>, bool> = true>
	static WindowType* Open(Args&&... args);
	static void Close(EditorWindow* wnd);

	static void UpdateAll();
};

template <typename WindowType, typename ... Args, std::enable_if_t<std::is_base_of_v<EditorWindow, WindowType>, bool>>
WindowType* EditorWindow::Open(Args&&... args)
{
	auto window = Unique<EditorWindow>(new WindowType(std::forward<Args>(args)...));
	return static_cast<WindowType*>(Register(std::move(window)));
}
