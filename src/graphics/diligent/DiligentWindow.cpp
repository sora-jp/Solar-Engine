#include "DiligentWindow.h"
#include "RenderTexture.h"

void DiligentResizeWindowCallback(GLFWwindow* window, int width, int height)
{
	auto owner = static_cast<DiligentWindow*>(glfwGetWindowUserPointer(window));
	owner->Resize(width, height);
	owner->InvalidateCachedRenderTarget();
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
		swapChainDesc.IsPrimary = isMainWindow;
		swapChainDesc.Width = w;
		swapChainDesc.Height = h;
		m_ctx->CreateSwapChain(swapChainDesc, glfwGetWin32Window(window), &m_swapchain);
	}
	else m_swapchain = swapChain;
	
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, DiligentResizeWindowCallback);
}

void DiligentWindow::Present(const int vsyncInterval)
{
	m_swapchain->Present(vsyncInterval);
}

void DiligentWindow::Resize(const int width, const int height)
{
	auto& desc = m_swapchain->GetDesc();
	if (desc.Width != width || desc.Height != height) 
	{
		m_renderTarget.reset();
		m_swapchain->Resize(width, height);
	}
}

void DiligentWindow::GetSize(int& width, int& height) const
{
	glfwGetWindowSize(m_window, &width, &height);
}

RenderTexture& DiligentWindow::GetRenderTarget()
{
	//if (!m_renderTarget) 
	{
		auto* colorBuf = m_swapchain->GetCurrentBackBufferRTV();
		auto* depthBuf = m_swapchain->GetDepthBufferDSV();

		if (m_renderTarget) m_renderTarget->Rebind(1, &colorBuf, depthBuf);
		else m_renderTarget = MakeUnique<RenderTexture>(1, &colorBuf, depthBuf);
	}
	return *m_renderTarget;
}

void DiligentWindow::InvalidateCachedRenderTarget()
{
	//m_renderTarget.reset();
}
