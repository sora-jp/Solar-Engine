#pragma once

#include <diligent/Graphics/GraphicsTools/interface/MapHelper.hpp>

#include "core/Common.h"
#include "TextureBase.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderConstants.h"
#include "diligent/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "diligent/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "diligent/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "diligent/Graphics/GraphicsTools/interface/ScopedQueryHelper.hpp"
#include "diligent/Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "Texture.h"

using namespace Diligent;

class DiligentWindow;
struct GLFWwindow;

//TODO: Expose a better low-level API

class DiligentContext : public std::enable_shared_from_this<DiligentContext> {
	static const RESOURCE_STATE_TRANSITION_MODE TRANSITION_MODE = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

	ShaderConstants m_constants;
	RenderTarget* m_activeTexture;
	QueryDataPipelineStatistics m_pipelineStats;
	double m_duration = 0;
	
	RefCntAutoPtr<IEngineFactory> m_factory;
	RefCntAutoPtr<IRenderDevice>  m_device;
	RefCntAutoPtr<IDeviceContext> m_context;
	RefCntAutoPtr<IBuffer> m_constantsBuf;
	std::unique_ptr<ScopedQueryHelper>	  m_statsQuery;
	std::unique_ptr<DurationQueryHelper>	  m_timerQuery;
	RENDER_DEVICE_TYPE            m_deviceType = static_cast<RENDER_DEVICE_TYPE>(-1);

	void TransitionState(IDeviceObject* obj, RESOURCE_STATE newState);
	void TransitionState(RenderTarget* tex, RESOURCE_STATE newColorState, RESOURCE_STATE newDepthState);

public:
	Shared<DiligentWindow> Init(GLFWwindow* window);

	void BeginFrame();
	void EndFrame();
	
	void SetRenderTarget(RenderTarget* texture, bool autoTransition = false);
	void Clear(float* rgba, float depth, uint8_t stencil, bool autoTransition = false);
	void FlushConstants() { *MapHelper<ShaderConstants>(m_context, m_constantsBuf, MAP_WRITE, MAP_FLAG_DISCARD) = m_constants; }
	
	void BindMaterial(const Shared<Material>& material, int subpass = 0);
	void SubmitMesh(const Shared<Mesh>& mesh, int subMesh);
	
	void Blit(RenderTarget* dest, const Shared<Material>& mat, int subpass = 0);
	void Blit(Texture* src, RenderTarget* dest, const Shared<Material>& mat, int subpass = 0);
	
	void DispatchCompute(glm::ivec3 groups, const Shared<Shader>& computeShader, MaterialPropertyBlock& mpb);
	void UnbindVertexBuffers();


	/*
	BeginFrame
	EndFrame

	SetRenderTarget
	Clear

	BindMaterial
	SubmitMesh
	*/

	void CreateSwapChain(const SwapChainDesc& desc, void* windowHandle, ISwapChain** outSwapChain);
	ITexture* CreateTexture(uint32_t width, uint32_t height, TEXTURE_FORMAT format, Diligent::BIND_FLAGS bindFlags, uint32_t mipLevels = 1, uint32_t msaa = 1);
	
	const QueryDataPipelineStatistics& GetPipelineStats() const { return m_pipelineStats; }
	const double& GetLastDuration() const { return m_duration; }
	
	template<typename T, std::enable_if_t<std::is_base_of_v<IEngineFactory, T>, bool> = true>
	[[nodiscard]] T* GetFactory() { return static_cast<T*>(m_factory.RawPtr()); }

	[[nodiscard]] auto GetDeviceType() const { return m_deviceType; }
	[[nodiscard]] IRenderDevice* GetDevice() { return m_device; }
	[[nodiscard]] IDeviceContext* GetContext() { return m_context; }
	[[nodiscard]] ShaderConstants* GetConstants() { return &m_constants; }
	[[nodiscard]] RefCntAutoPtr<IBuffer> GetConstantBuffer() const { return m_constantsBuf; }
	void ResolveMSAA(RenderTarget* source, RenderTarget* dest);
};
