#pragma once

#include "core/Common.h"
#include "core/Subsystem.h"
#include "diligent/DiligentInit.h"

struct GLFWwindow;
class SUBSYSTEM_API GraphicsSubsystem final : Subsystem
{
	SUBSYSTEM_NAME("Graphics Subsystem");
	SUBSYSTEM_ORDER(-1);

	Shared<DiligentContext> m_ctx;
	GLFWwindow* m_window = nullptr;
	
public:
	~GraphicsSubsystem() override;
	void Init() override;
	void Shutdown() override;
	void PostRun() override;
	void PreRun() override;

	bool RequestedShutdown() override;
};