#pragma once
#include "core/Common.h"
#include "RenderTexture.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "diligent/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

using namespace Diligent;

class DiligentWindow
{
	GLFWwindow* m_window;
	RefCntAutoPtr<ISwapChain> m_swapchain;
	Shared<DiligentContext> m_ctx;
	Unique<RenderTexture> m_renderTarget;

public:
	DiligentWindow(const Shared<DiligentContext>& ctx, bool isMainWindow, GLFWwindow* window);
	DiligentWindow(const Shared<DiligentContext>& ctx, ISwapChain* swapChain, bool isMainWindow, GLFWwindow* window);
	~DiligentWindow() noexcept = default;

	void Present();
	void Resize(int width, int height);
	void GetSize(int& width, int& height) const;
	[[nodiscard]] RenderTexture& GetRenderTarget();

	Shared<DiligentContext> GetContext() { return m_ctx; }
	ISwapChain* GetSwapChain() { return m_swapchain; }
	operator GLFWwindow*() const { return m_window; }
	void InvalidateCachedTexture();
};