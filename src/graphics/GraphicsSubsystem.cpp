#include "pch.h"

#include "GraphicsSubsystem.h"
#include "core/Log.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bgfx-imgui/imgui.h"
#include "dear-imgui/backends/imgui_impl_glfw.h"

#define WNDW_WIDTH 1280
#define WNDW_HEIGHT 720

GraphicsSubsystem::~GraphicsSubsystem() = default;

static std::mutex _imGuiMutex;
static std::thread _imGuiThread;
static double _scrollDelta;
static volatile bool _shuttingDown = false;

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
    _scrollDelta += yoffset;
}

//void imGuiThread()
//{
//	SOLAR_CORE_TRACE("Init ImGui thread");
//	try {
//		while (!_shuttingDown)
//		{
//			_imGuiMutex.lock();
//			_imGuiMutex.unlock();
//
//			std::this_thread::yield();
//		}
//		
//		SOLAR_CORE_TRACE("ImGui thread shutting down");
//	}
//	catch (const std::exception& e)
//	{
//		_imGuiMutex.unlock();
//		SOLAR_CORE_ERROR(e.what());
//	}
//}

void SetupImGuiStyle2();
void GraphicsSubsystem::Init()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Init()");

	const auto glfwInitResult = glfwInit();
	SOLAR_CORE_ASSERT_ALWAYS(glfwInitResult == GLFW_TRUE);

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "Hello, bgfx!", nullptr, nullptr);
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	//if (glfwRawMouseMotionSupported()) glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetScrollCallback(m_window, &scroll_callback);
	
    bgfx::PlatformData pd;
    pd.nwh = glfwGetWin32Window(m_window);
	
    bgfx::Init bgfxInit;
	bgfxInit.platformData = pd;
	bgfxInit.type = bgfx::RendererType::OpenGL; // Automatically choose a renderer.
	bgfxInit.debug = false;
	bgfxInit.resolution.maxFrameLatency = 0;
	bgfxInit.resolution.numBackBuffers = 1;
    bgfxInit.resolution.width = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset = BGFX_RESET_NONE;

	//bgfx::renderFrame();
	
	const auto bgfxInitSuccess = init(bgfxInit);
	SOLAR_CORE_ASSERT_ALWAYS(bgfxInitSuccess);

	SOLAR_CORE_INFO("Initialized {}", bgfx::getRendererName(bgfx::getRendererType()));
	
	//bgfx::setDebug(BGFX_DEBUG_IFH);
	
	imguiCreate();
	ImGui_ImplGlfw_InitForOther(m_window, true);
	SetupImGuiStyle2();

	//ImGui::GetIO().MouseDrawCursor = true;
	//_imGuiThread = std::thread(imGuiThread);
}

void GraphicsSubsystem::Shutdown()
{
	SOLAR_CORE_TRACE("GraphicsSubsystem::Shutdown()");
	_shuttingDown = true;
	//_imGuiThread.join();
	glfwDestroyWindow(m_window);
	ImGui_ImplGlfw_Shutdown();
	imguiDestroy();
	bgfx::shutdown();
	glfwTerminate();
}

void GraphicsSubsystem::PreRun()
{
	_imGuiMutex.lock();
	
	ImGui::UpdatePlatformWindows();
	
	static auto lastFrameTime = std::chrono::high_resolution_clock::now();
	const auto curFrameTime = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double> deltaSeconds = curFrameTime - lastFrameTime;
	lastFrameTime = curFrameTime;

	ImGui::GetIO().DeltaTime = deltaSeconds.count();
	
	double mx, my;
	glfwGetCursorPos(m_window, &mx, &my);
    const uint8_t mouseBtns =
		  glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)
        | glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) << 1
		| glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) << 2;

	ImGui_ImplGlfw_NewFrame();
	imguiBeginFrame();
	ImGui::NewFrame();
	
	//imguiBeginFrame(m_window, deltaSeconds.count(), mx, my, mouseBtns, _scrollDelta, WNDW_WIDTH, WNDW_HEIGHT, -1, 0);
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
	
	bgfx::setDebug(BGFX_DEBUG_TEXT | (debugStats ? BGFX_DEBUG_STATS : 0) | (debugProfiler ? BGFX_DEBUG_WIREFRAME : 0));
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);
	bgfx::touch(0);
	
	bgfx::setDebug(BGFX_DEBUG_TEXT | (debugStats ? BGFX_DEBUG_STATS : 0));
	bgfx::dbgTextClear();

	ImGui::Render();
	imguiRender(ImGui::GetMainViewport(), 0);

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	bgfx::frame();
	
	glfwPollEvents();
	_imGuiMutex.unlock();
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