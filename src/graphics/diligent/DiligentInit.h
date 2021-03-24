#pragma once

#include "core/Common.h"
#include "diligent/RenderTexture.h"
#include "diligent/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "diligent/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "diligent/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "diligent/Graphics/GraphicsTools/interface/ScopedQueryHelper.hpp"
#include "diligent/Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

class DiligentWindow;
struct GLFWwindow;

class DiligentContext : public std::enable_shared_from_this<DiligentContext> {
	static const RESOURCE_STATE_TRANSITION_MODE TRANSITION_MODE = RESOURCE_STATE_TRANSITION_MODE_VERIFY;
	
	RenderTexture m_activeTexture {};
	QueryDataPipelineStatistics m_pipelineStats;
	double m_duration = 0;
	
	RefCntAutoPtr<IEngineFactory> m_factory;
	RefCntAutoPtr<IRenderDevice>  m_device;
	RefCntAutoPtr<IDeviceContext> m_context;
	std::unique_ptr<ScopedQueryHelper>	  m_statsQuery;
	std::unique_ptr<DurationQueryHelper>	  m_timerQuery;
	RENDER_DEVICE_TYPE            m_deviceType = static_cast<RENDER_DEVICE_TYPE>(-1);

	void TransitionState(IDeviceObject* obj, RESOURCE_STATE newState);
	void TransitionState(RenderTexture* tex, RESOURCE_STATE newColorState, RESOURCE_STATE newDepthState);

public:
	Shared<DiligentWindow> Init(GLFWwindow* window);

	void BeginFrame();
	void CreateSwapChain(const SwapChainDesc& desc, void* windowHandle, ISwapChain** outSwapChain);
	void SetRenderTarget(RenderTexture& texture, bool autoTransition = true);
	void Clear(float* rgba, float depth, uint8_t stencil, bool autoTransition = true);
	void EndFrame();

	const QueryDataPipelineStatistics& GetPipelineStats() const { return m_pipelineStats; }
	const double& GetLastDuration() const { return m_duration; }
	
	template<typename T, std::enable_if_t<std::is_base_of_v<IEngineFactory, T>, bool> = true>
	[[nodiscard]] T* GetFactory() { return static_cast<T*>(m_factory.RawPtr()); }

	[[nodiscard]] auto GetDeviceType() const { return m_deviceType; }
	[[nodiscard]] IRenderDevice* GetDevice() { return m_device; }
	[[nodiscard]] IDeviceContext* GetContext() { return m_context; }
};
