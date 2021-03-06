#pragma once

#include "core/Common.h"
#include "core/Subsystem.h"
#include "PipelineStats.h"

struct GLFWwindow;
class DiligentContext;
class DiligentWindow;

class SUBSYSTEM_API GraphicsSubsystem final : public Subsystem
{
	SUBSYSTEM_NAME("Graphics Subsystem");
	SUBSYSTEM_ORDER(-1);

	static Shared<DiligentWindow> _mainWindow;
	static Shared<DiligentContext> _ctx;
	GLFWwindow* m_window = nullptr;
	
public:
	~GraphicsSubsystem() override;
	void Init() override;
	void Shutdown() override;
	void PostRun() override;
	void PreRun() override;
	
	static Shared<DiligentContext> GetContext() { return _ctx; }
	static Shared<DiligentWindow> GetMainWindow() { return _mainWindow; }
	static const PipelineStats& GetStats();
	static const double& GetLastDuration();
	
	bool RequestedShutdown() override;
};