#pragma once
#include "core/Common.h"
#include "TextureBase.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "diligent/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"

using namespace Diligent;

class DiligentWindow
{
	GLFWwindow* m_window;
	RefCntAutoPtr<ISwapChain> m_swapchain;
	Shared<DiligentContext> m_ctx;
	RenderTarget* m_renderTarget;

public:
	DiligentWindow(const Shared<DiligentContext>& ctx, bool isMainWindow, GLFWwindow* window);
	DiligentWindow(const Shared<DiligentContext>& ctx, ISwapChain* swapChain, bool isMainWindow, GLFWwindow* window);
	~DiligentWindow() noexcept { glfwDestroyWindow(m_window); }

	void Present(int vsyncInterval = 1);
	void Resize(int width, int height);
	void GetSize(int& width, int& height) const;
	[[nodiscard]] RenderTarget* GetRenderTarget() const;

	Shared<DiligentContext> GetContext() const { return m_ctx; }
	ISwapChain* GetSwapChain() { return m_swapchain; }
	operator GLFWwindow*() const { return m_window; }
};