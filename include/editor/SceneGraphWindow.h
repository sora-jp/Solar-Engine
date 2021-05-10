#pragma once

#include "EditorWindow.h"

class SceneGraphWindow final : public EditorWindow
{
protected:
	std::string Title() const override;
public:
	void Draw() override;
};