#include "pch.h"

#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"

#include "RenderSystem.h"

#include "core/Engine.h"
#include "core/Profiler.h"

#include "diligent/DiligentWindow.h"

#include "diligent-imgui/ImGuiDiligentRenderer.h"
#include "imbackends/imgui_impl_glfw.h"
#include "ImGuiStyle.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "core/Input.h"

#include "pipeline/impl/SimplePipeline.h"
#include "glslang/Public/ShaderLang.h"

#include "ImGuizmo.h"

#define WNDW_WIDTH 1920
#define WNDW_HEIGHT 1080

Shared<DiligentContext> GraphicsSubsystem::_ctx;
Shared<DiligentWindow> GraphicsSubsystem::_mainWindow;

GraphicsSubsystem::~GraphicsSubsystem() = default;

static ImGuiDiligentRenderer* s_imguiRenderer;
static ImFont *s_monoFont, *s_monoFontSmall;
static Shared<RenderSystem> s_renderSystem;
static double s_time;

void GraphicsSubsystem::Init()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Init()");
	glslang::InitializeProcess();

	const auto glfwInitResult = glfwInit();
	SOLAR_CORE_ASSERT_ALWAYS(glfwInitResult == GLFW_TRUE);

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	m_window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "Hello, Diligent!", nullptr, nullptr);
	
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
	if (glfwRawMouseMotionSupported()) glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	_ctx = MakeShared<DiligentContext>();
	_mainWindow = _ctx->Init(m_window);

	Input::AddDevice<Keyboard>(m_window);
	Input::AddDevice<Mouse>(m_window);

	auto& devcaps = _ctx->GetDevice()->GetDeviceCaps();
	
	SOLAR_CORE_INFO("Using GPU {} ({})", _ctx->GetDevice()->GetDeviceCaps().AdapterInfo.DeviceId, devcaps.AdapterInfo.Description);

	auto& ndc = devcaps.GetNDCAttribs();

	SOLAR_CORE_INFO("Depth bias: {}, Min depth: {}, Z to Depth scale: {}, Y to V scale: {}", ndc.GetZtoDepthBias(), ndc.MinZ, ndc.ZtoDepthScale, ndc.YtoVScale);
	
	ImGui::CreateContext();
	ImPlot::CreateContext();

	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	
	io.FontDefault  = io.Fonts->AddFontFromMemoryTTF(Montserrat_Regular_ttf, 64, 16.0f);
	s_monoFont      = io.Fonts->AddFontFromMemoryTTF(FiraCode_Regular_ttf, 64, 16.0f);
	s_monoFontSmall = io.Fonts->AddFontFromMemoryTTF(FiraCode_Regular_ttf, 64, 12.0f);
	
	const auto& scd = _mainWindow->GetSwapChain()->GetDesc();
	s_imguiRenderer = new ImGuiDiligentRenderer(_ctx->GetDevice(), scd.ColorBufferFormat, scd.DepthBufferFormat, 1024, 1024);

	s_renderSystem = MakeShared<RenderSystem>();
	s_renderSystem->SetContext(PipelineContext(_ctx));
	s_renderSystem->SetPipeline(MakeUnique<SimplePipeline>());
	
	ImGui_ImplGlfw_InitForOther(m_window, true);
	SetupImGuiStyle();
}

void GraphicsSubsystem::Shutdown()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Shutdown()");
	
	ImGui_ImplGlfw_Shutdown();
	delete s_imguiRenderer;

	_mainWindow.reset();
	_ctx.reset();
	
	glfwTerminate();
	glslang::FinalizeProcess();
}

void GraphicsSubsystem::PreRun()
{
	static auto lastFrameTime = std::chrono::high_resolution_clock::now();
	const auto curFrameTime = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double> deltaSeconds = curFrameTime - lastFrameTime;
	lastFrameTime = curFrameTime;
	s_time += deltaSeconds.count();

	ImGui_ImplGlfw_NewFrame();
	
	//TODO: Diligent new imgui frame

	int w, h;
	_mainWindow->GetSize(w, h);
	s_imguiRenderer->NewFrame(w, h, SURFACE_TRANSFORM_IDENTITY);
	
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	Input::UpdateAll();
}

void GraphicsSubsystem::PostRun()
{
	//static auto debugStats = true;
	//static auto lastState = GLFW_RELEASE;
	//
	//const auto state = glfwGetKey(m_window, GLFW_KEY_F11);
	//
	//if (state == GLFW_PRESS && lastState == GLFW_RELEASE) debugStats = !debugStats;
	//lastState = state;

	//if (debugStats)
	//{
	//	DrawDebugWindow(_ctx, s_monoFont, s_monoFontSmall);
	//}

	//TODO: Render
	s_imguiRenderer->EndFrame();
	ImGui::Render();
	
	ImGui::UpdatePlatformWindows();

	Profiler::Begin("Render", "Rendering");
	_ctx->BeginFrame();

	_ctx->SetRenderTarget(_mainWindow->GetRenderTarget());
	_ctx->Clear(nullptr, 1.0f, 0);

	s_renderSystem->SetTarget(_mainWindow->GetRenderTarget());
	Engine::RunEcsSystem(s_renderSystem);
	
	_ctx->SetRenderTarget(_mainWindow->GetRenderTarget());
	s_imguiRenderer->RenderDrawData(_ctx->GetContext(), ImGui::GetDrawData());

	_ctx->EndFrame();
	Profiler::End();

	Profiler::Begin("VSync", "VSync");
	_mainWindow->Present(0);
	Profiler::End();
	
	glfwPollEvents();
}

const PipelineStats& GraphicsSubsystem::GetStats()
{
	return *reinterpret_cast<const PipelineStats*>(&_ctx->GetPipelineStats());
}

const double& GraphicsSubsystem::GetLastDuration()
{
	return _ctx->GetLastDuration();
}

bool GraphicsSubsystem::RequestedShutdown()
{
	return glfwWindowShouldClose(m_window);
}
