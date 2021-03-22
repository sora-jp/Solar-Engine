#include "pch.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "diligent/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "diligent/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "diligent/Platforms/Win32/interface/Win32NativeWindow.h"

#include "core/Log.h"

void DiligentContext::Init(void* nwh)
{
	if (m_deviceType == -1) m_deviceType = RENDER_DEVICE_TYPE_D3D11;
	
	SwapChainDesc swapchain;
	swapchain.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM;
	swapchain.IsPrimary = true;
	
	FullScreenModeDesc fullscreen;
	Win32NativeWindow window{nwh};

	switch (m_deviceType)
	{
		case RENDER_DEVICE_TYPE_D3D11:
		{
			EngineD3D11CreateInfo d3d11Info;
			auto* d3d11Factory = LoadGraphicsEngineD3D11()();

			d3d11Factory->CreateDeviceAndContextsD3D11(d3d11Info, &m_device, &m_context);
			d3d11Factory->CreateSwapChainD3D11(m_device, m_context, swapchain, fullscreen, window, &m_swapChain);
			break;
		}

		case RENDER_DEVICE_TYPE_D3D12:
		{
			EngineD3D12CreateInfo d3d12Info;
			auto* d3d12Factory = LoadGraphicsEngineD3D12()();

			d3d12Factory->CreateDeviceAndContextsD3D12(d3d12Info, &m_device, &m_context);
			d3d12Factory->CreateSwapChainD3D12(m_device, m_context, swapchain, fullscreen, window, &m_swapChain);
			break;
		}

		case RENDER_DEVICE_TYPE_GL:
		{
			EngineGLCreateInfo glInfo;
			glInfo.Window.hWnd = nwh;

			auto* glFactory = LoadGraphicsEngineOpenGL()();
			glFactory->CreateDeviceAndSwapChainGL(glInfo, &m_device, &m_context, swapchain, &m_swapChain);
			break;
		}

		case RENDER_DEVICE_TYPE_VULKAN:
		{
			EngineVkCreateInfo vulkanInfo;
			auto* vulkanFactory = LoadGraphicsEngineVk()();

			vulkanFactory->CreateDeviceAndContextsVk(vulkanInfo, &m_device, &m_context);
			vulkanFactory->CreateSwapChainVk(m_device, m_context, swapchain, window, &m_swapChain);
			break;
		}

		default:
			SOLAR_CORE_CRITICAL("Unknown graphics device type");
			SOLAR_CORE_ASSERT(0);
			break;
	}
}
