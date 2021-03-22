#pragma once

#include "core/Common.h"
#include "diligent/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "diligent/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "diligent/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

class DiligentContext {
	RefCntAutoPtr<IRenderDevice>  m_device;
	RefCntAutoPtr<IDeviceContext> m_context;
	RefCntAutoPtr<ISwapChain>     m_swapChain;
	RENDER_DEVICE_TYPE            m_deviceType = static_cast<RENDER_DEVICE_TYPE>(-1);

public:
	void Init(void* nwh);
	[[nodiscard]] auto GetDeviceType() const { return m_deviceType; }
	[[nodiscard]] auto& GetDevice() { return m_device; }
	[[nodiscard]] auto& GetContext() { return m_context; }
	[[nodiscard]] auto& GetMainSwapChain() { return m_swapChain; }
};
