#pragma once
#include "EditorWindow.h"
#include "core/ComponentWrapper.h"
#include "core/Entity.h"

class InspectorWindow final : public EditorWindow
{
protected:
	[[nodiscard]] std::string Title() const override;
public:
	void Draw() override;
};