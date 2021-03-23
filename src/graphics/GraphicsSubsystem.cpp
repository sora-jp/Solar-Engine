#include "pch.h"
#include "fonts/Montserrat_Regular.ttf.h"

#include "GraphicsSubsystem.h"
#include "core/Log.h"

#include "diligent/DiligentWindow.h"
#include "diligent-imgui/ImGuiDiligentRenderer.h"
#include "GLFW/glfw3.h"
#include "dear-imgui/backends/imgui_impl_glfw.h"
#include "diligent/ScopedRendering.h"

#define WNDW_WIDTH 1280
#define WNDW_HEIGHT 720

GraphicsSubsystem::~GraphicsSubsystem() = default;
static double _scrollDelta;

void ScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
    _scrollDelta += yoffset;
}

void SetupImGuiStyle2();
static ImGuiDiligentRenderer* s_imguiRenderer;
static Shared<DiligentWindow> s_window;

void GraphicsSubsystem::Init()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Init()");

	const auto glfwInitResult = glfwInit();
	SOLAR_CORE_ASSERT_ALWAYS(glfwInitResult == GLFW_TRUE);

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	m_window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "Hello, bgfx!", nullptr, nullptr);

	glfwSetScrollCallback(m_window, &ScrollCallback);

	m_ctx = MakeShared<DiligentContext>();
	s_window = m_ctx->Init(m_window);
	
	SOLAR_CORE_INFO("Using GPU {} ({})", m_ctx->GetDevice()->GetDeviceCaps().AdapterInfo.DeviceId, m_ctx->GetDevice()->GetDeviceCaps().AdapterInfo.Description);

	auto* ctx = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx);

	auto& io = ImGui::GetIO();
	io.FontDefault = io.Fonts->AddFontFromMemoryTTF(Montserrat_Regular_ttf, 18, 18.0f);

	const auto& scd = s_window->GetSwapChain()->GetDesc();
	s_imguiRenderer = new ImGuiDiligentRenderer(m_ctx->GetDevice(), scd.ColorBufferFormat, scd.DepthBufferFormat, 1024, 1024);
	
	ImGui_ImplGlfw_InitForOther(m_window, true);
	SetupImGuiStyle2();
}

void GraphicsSubsystem::Shutdown()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Shutdown()");
	
	ImGui_ImplGlfw_Shutdown();
	delete s_imguiRenderer;
	
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void GraphicsSubsystem::PreRun()
{
	ImGui::UpdatePlatformWindows();
	
	static auto lastFrameTime = std::chrono::high_resolution_clock::now();
	const auto curFrameTime = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double> deltaSeconds = curFrameTime - lastFrameTime;
	lastFrameTime = curFrameTime;

	ImGui_ImplGlfw_NewFrame();
	ImGui::GetIO().DeltaTime = deltaSeconds.count();
	
	//TODO: Diligent new imgui frame

	int w, h;
	s_window->GetSize(w, h);
	s_imguiRenderer->NewFrame(w, h, SURFACE_TRANSFORM_IDENTITY);
	ImGui::NewFrame();
	
	_scrollDelta = 0;
	//ImGui::RootDock(ImVec2(0, 0), ImVec2(WNDW_WIDTH, WNDW_HEIGHT));
}

void GraphicsSubsystem::PostRun()
{
	//SOLAR_CORE_TRACE("GraphicsSubsystem::Run()");
	static auto debugStats = false;
	static auto lastState = GLFW_RELEASE;
	
	const auto state = glfwGetKey(m_window, GLFW_KEY_F11);
	
	if (state == GLFW_PRESS && lastState == GLFW_RELEASE) debugStats = !debugStats;
	lastState = state;

	static auto debugProfiler = false;
	static auto lastStateP = GLFW_RELEASE;
	const auto stateP = glfwGetKey(m_window, GLFW_KEY_F10);

	if (stateP == GLFW_PRESS && lastStateP == GLFW_RELEASE) debugProfiler = !debugProfiler;
	lastStateP = stateP;

	//TODO: Render
	s_imguiRenderer->EndFrame();
	ImGui::Render();

	const auto main = ScopedRenderingContext::Begin(s_window);
	
	main->BindRenderTarget();
	main->Clear(nullptr, 1.0f, 0);
	
	s_imguiRenderer->RenderDrawData(m_ctx->GetContext(), ImGui::GetDrawData());
	
	main->End();

	m_ctx->GetContext()->Flush();
	s_window->Present();
	
	glfwPollEvents();
}

bool GraphicsSubsystem::RequestedShutdown()
{
	return glfwWindowShouldClose(m_window);
}

void SetupImGuiStyle2()
{
	auto style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(10, 10);
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(8, 8);
	style->ItemInnerSpacing = ImVec2(8, 8);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 12.0f;
	style->GrabMinSize = 6.0f;
	
	style->WindowRounding = 0.0f;
	style->ChildRounding = 2.0f;
	style->GrabRounding = 2.0f;
	style->FrameRounding = 2.0f;
	style->PopupRounding = 2.0f;
	style->TabRounding = 2.0f;
	style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	
	style->WindowBorderSize = style->ChildBorderSize = style->FrameBorderSize = style->TabBorderSize = 0;
	style->PopupBorderSize = 1;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.33f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.20f, 0.20f, 0.23f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 0.70f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	style->Colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.36f, 0.36f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
}

EXPORT_SUBSYSTEM(GraphicsSubsystem)