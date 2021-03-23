#include "pch.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "diligent/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "diligent/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "diligent/Platforms/Win32/interface/Win32NativeWindow.h"

#include "core/Log.h"
#include "diligent/DiligentWindow.h"
#include "diligent/RenderTexture.h"

Shared<DiligentWindow> DiligentContext::Init(GLFWwindow* window)
{
	if (m_deviceType == -1) m_deviceType = RENDER_DEVICE_TYPE_D3D11;
	
	SwapChainDesc swapchain;
	swapchain.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM;
	swapchain.IsPrimary = true;

	switch (m_deviceType)
	{
		case RENDER_DEVICE_TYPE_D3D11:
		{
			EngineD3D11CreateInfo d3d11Info;
			auto* d3d11Factory = LoadGraphicsEngineD3D11()();
			m_factory = d3d11Factory;
			
			d3d11Factory->CreateDeviceAndContextsD3D11(d3d11Info, &m_device, &m_context);
			break;
		}

		case RENDER_DEVICE_TYPE_D3D12:
		{
			EngineD3D12CreateInfo d3d12Info;
			auto* d3d12Factory = LoadGraphicsEngineD3D12()();
			m_factory = d3d12Factory;

			d3d12Factory->CreateDeviceAndContextsD3D12(d3d12Info, &m_device, &m_context);
			break;
		}

		case RENDER_DEVICE_TYPE_GL:
		{
			EngineGLCreateInfo glInfo;
			glInfo.Window.hWnd = window;

			auto* glFactory = LoadGraphicsEngineOpenGL()();
			m_factory = glFactory;

			ISwapChain* tmp;
			glFactory->CreateDeviceAndSwapChainGL(glInfo, &m_device, &m_context, swapchain, &tmp);
			return MakeShared<DiligentWindow>(shared_from_this(), tmp, true, window);
		}

		case RENDER_DEVICE_TYPE_VULKAN:
		{
			EngineVkCreateInfo vulkanInfo;
			auto* vulkanFactory = LoadGraphicsEngineVk()();
			m_factory = vulkanFactory;

			vulkanFactory->CreateDeviceAndContextsVk(vulkanInfo, &m_device, &m_context);
			break;
		}

		default:
			SOLAR_CORE_CRITICAL("Unknown graphics device type");
			SOLAR_CORE_ASSERT(0);
			break;
	}

	return MakeShared<DiligentWindow>(shared_from_this(), true, window);
}

void DiligentContext::CreateSwapChain(const SwapChainDesc& desc, void* windowHandle, ISwapChain** outSwapChain)
{
	const FullScreenModeDesc fsMode;
	const Win32NativeWindow wnd { windowHandle };
	
	switch (m_device->GetDeviceCaps().DevType)
	{
		case RENDER_DEVICE_TYPE_GL:
		case RENDER_DEVICE_TYPE_GLES:
			SOLAR_CORE_WARN("OpenGL[ES] devices do not currently support multiple swapchains");
			break;
		case RENDER_DEVICE_TYPE_D3D11:
			GetFactory<IEngineFactoryD3D11>()->CreateSwapChainD3D11(m_device, m_context, desc, fsMode, wnd, outSwapChain);
			break;
		case RENDER_DEVICE_TYPE_D3D12:
			GetFactory<IEngineFactoryD3D12>()->CreateSwapChainD3D12(m_device, m_context, desc, fsMode, wnd, outSwapChain);
			break;
		case RENDER_DEVICE_TYPE_VULKAN:
			GetFactory<IEngineFactoryVk>()->CreateSwapChainVk(m_device, m_context, desc, wnd, outSwapChain);
			break;
		default:
			SOLAR_CORE_CRITICAL("Invalid device type {}", m_device->GetDeviceCaps().DevType);
			SOLAR_CORE_ASSERT_ALWAYS(0);
			break;
	}
}

void DiligentContext::SetRenderTarget(RenderTexture& texture)
{
	m_context->SetRenderTargets(texture.m_numColorTargets, texture.m_rawColorTargets, texture.m_depthTarget, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void DiligentContext::ClearRenderTexture(RenderTexture& texture, float* rgba, float depth, uint8_t stencil)
{
	if (texture.m_depthTarget) 
		m_context->ClearDepthStencil(texture.m_depthTarget, CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, depth, stencil, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	for (auto i = 0; i < texture.m_numColorTargets; i++)
	{
		m_context->ClearRenderTarget(texture.m_colorTargets[i], rgba, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
}