#include "pch.h"
#include "DiligentWindow.h"
#include <GLFW/glfw3native.h>

class WindowRenderTarget final : RenderTargetBase
{
	RefCntWeakPtr<ISwapChain> m_swapchain;
	ITextureView* m_curBackbuffer;	

	[[nodiscard]] size_t GetColorTargetCount() override { return 1; }
	[[nodiscard]] Diligent::ITextureView** GetColorTargets() override
	{
		m_curBackbuffer = m_swapchain.IsValid() ? m_swapchain.Lock()->GetCurrentBackBufferRTV() : nullptr;
		return &m_curBackbuffer;
	}
	[[nodiscard]] Diligent::ITextureView* GetDepthTarget() override { return m_swapchain.IsValid() ? m_swapchain.Lock()->GetDepthBufferDSV() : nullptr; }

public:
	WindowRenderTarget(ISwapChain* swapChain) : m_swapchain(swapChain), m_curBackbuffer(swapChain->GetCurrentBackBufferRTV()) {}
};

void DiligentResizeWindowCallback(GLFWwindow* window, int width, int height)
{
	auto* owner = static_cast<DiligentWindow*>(glfwGetWindowUserPointer(window));
	owner->Resize(width, height);
}

DiligentWindow::DiligentWindow(const Shared<DiligentContext>& ctx, const bool isMainWindow, GLFWwindow* window) : DiligentWindow(ctx, nullptr, isMainWindow, window) {}
DiligentWindow::DiligentWindow(const Shared<DiligentContext>& ctx, ISwapChain* swapChain, const bool isMainWindow, GLFWwindow* window)
{
	m_ctx = ctx;
	m_window = window;
	if (swapChain == nullptr) 
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		
		SwapChainDesc swapChainDesc;
		swapChainDesc.PreTransform = SURFACE_TRANSFORM_OPTIMAL;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM_SRGB;
		swapChainDesc.IsPrimary = isMainWindow;
		swapChainDesc.Width = w;
		swapChainDesc.Height = h;
		
		m_ctx->CreateSwapChain(swapChainDesc, glfwGetWin32Window(window), &m_swapchain);
	}
	else m_swapchain = swapChain;

	m_renderTarget = Unique<RenderTarget>(reinterpret_cast<RenderTarget*>(new WindowRenderTarget(m_swapchain)));
	
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, DiligentResizeWindowCallback);
}

void DiligentWindow::Present(const int vsyncInterval)
{
	m_swapchain->Present(vsyncInterval);
}

void DiligentWindow::Resize(const int width, const int height)
{
	const auto& desc = m_swapchain->GetDesc();
	if (desc.Width != width || desc.Height != height) 
	{
		m_swapchain->Resize(width, height);
	}
}

void DiligentWindow::GetSize(int& width, int& height) const
{
	glfwGetWindowSize(m_window, &width, &height);
}

RenderTarget* DiligentWindow::GetRenderTarget() const
{
	return m_renderTarget.get();
}