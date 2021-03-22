#pragma once

#include "core/Common.h"
#include "core/Subsystem.h"

struct GLFWwindow;
class SUBSYSTEM_API GraphicsSubsystem final : Subsystem
{
	SUBSYSTEM_NAME("Graphics subsystem (BGFX)");
	SUBSYSTEM_ORDER(-1);

	GLFWwindow* m_window = nullptr;
	
public:
	~GraphicsSubsystem() override;
	void Init() override;
	void Shutdown() override;
	void PostRun() override;
	void PreRun() override;

	bool RequestedShutdown() override;
};