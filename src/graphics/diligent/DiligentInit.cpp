#include "pch.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "diligent/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "diligent/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "diligent/Platforms/Win32/interface/Win32NativeWindow.h"

#include "core/Log.h"
#include "diligent/DiligentWindow.h"

#define TO_TEXVIEW(x) (static_cast<Diligent::ITextureView*>(x))
#define TO_TEXVIEW_ARR(x) (reinterpret_cast<Diligent::ITextureView**>(x))
#define TO_TEX(x) (static_cast<Diligent::ITexture*>(x))

void SetupTransitionDesc(StateTransitionDesc& transition, IDeviceObject* obj, const RESOURCE_STATE newState)
{
	SOLAR_CORE_ASSERT(obj != nullptr);
	
	transition.pResource = obj;
	transition.TransitionType = STATE_TRANSITION_TYPE_IMMEDIATE;
	transition.OldState = RESOURCE_STATE_UNKNOWN;
	transition.NewState = newState;
	transition.UpdateResourceState = true;
}

void AppendTransitionDesc(std::vector<StateTransitionDesc>& arr, IDeviceObject* obj, const RESOURCE_STATE newState)
{
	StateTransitionDesc d;
	SetupTransitionDesc(d, obj, newState);
	arr.push_back(d);
}

void DiligentContext::TransitionState(IDeviceObject* obj, const RESOURCE_STATE newState)
{
	StateTransitionDesc transition;
	SetupTransitionDesc(transition, obj, newState);
	
	m_context->TransitionResourceStates(1, &transition);
}

void DiligentContext::TransitionState(RenderTarget* tex, const RESOURCE_STATE newColorState, const RESOURCE_STATE newDepthState)
{
	//SOLAR_CORE_ASSERT(tex->IsValid());

	std::vector<StateTransitionDesc> transitions;

	//AppendTransitionDesc(transitions, tex->m_depthTarget, newDepthState);
	if (tex->depthRtv) AppendTransitionDesc(transitions, TO_TEXVIEW(tex->depthRtv)->GetTexture(), newDepthState);

	for (auto i = 0; i < tex->colorRtvs.size(); i++)
	{
		//AppendTransitionDesc(transitions, tex->m_colorTargets[i], newColorState);
		if (tex->colorRtvs[i]) AppendTransitionDesc(transitions, TO_TEXVIEW(tex->colorRtvs[i])->GetTexture(), newColorState);
	}

	m_context->TransitionResourceStates(transitions.size(), transitions.data());
}

Shared<DiligentWindow> DiligentContext::Init(GLFWwindow* window)
{
	if (m_deviceType == -1) m_deviceType = RENDER_DEVICE_TYPE_D3D11;

	Shared<DiligentWindow> outWindow;
	switch (m_deviceType)
	{
		case RENDER_DEVICE_TYPE_D3D11:
		{
			EngineD3D11CreateInfo d3d11Info;
			//d3d11Info.DebugFlags = D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE;
				
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

			//NOTE: Since OpenGL doesn't support separate swapchains, we need to shortcut the swapchain initialization below
			SwapChainDesc swapchain;
			swapchain.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM;
			swapchain.IsPrimary = true;
				
			ISwapChain* tmp;
			glFactory->CreateDeviceAndSwapChainGL(glInfo, &m_device, &m_context, swapchain, &tmp);

			outWindow = MakeShared<DiligentWindow>(shared_from_this(), tmp, true, window);
			break;
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
			SOLAR_CORE_DIE("Unknown/unsupported graphics device type");
	}

	QueryDesc statsQuery;
	statsQuery.Name = "Pipeline statistics query";
	statsQuery.Type = QUERY_TYPE_PIPELINE_STATISTICS;
	m_statsQuery.reset(new ScopedQueryHelper(m_device, statsQuery, 2));
	
	m_timerQuery.reset(new DurationQueryHelper(m_device, 2));

	BufferDesc buf;
	buf.Name = "Shader Constants";
	buf.Mode = BUFFER_MODE_RAW;
	buf.BindFlags = BIND_UNIFORM_BUFFER;
	buf.Usage = USAGE_DYNAMIC;
	buf.uiSizeInBytes = sizeof(ShaderConstants);
	buf.CPUAccessFlags = CPU_ACCESS_WRITE;

	m_device->CreateBuffer(buf, nullptr, &m_constantsBuf);
	
	return outWindow ? outWindow : MakeShared<DiligentWindow>(shared_from_this(), true, window);
}

void DiligentContext::BeginFrame()
{
	m_statsQuery->Begin(m_context);
	m_timerQuery->Begin(m_context);
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
			*outSwapChain = nullptr; // Surely this will kill it...
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
			SOLAR_CORE_DIE("Invalid device type {}", m_device->GetDeviceCaps().DevType);
	}
}

ITexture* DiligentContext::CreateTexture(uint32_t width, uint32_t height, TEXTURE_FORMAT format, Diligent::BIND_FLAGS bindFlags, uint32_t mipLevels, uint32_t msaa)
{
	TextureDesc texDesc;
	texDesc.Type = RESOURCE_DIM_TEX_2D;
	texDesc.Format = format;
	texDesc.Usage = USAGE_DEFAULT;
	texDesc.BindFlags = bindFlags;
	texDesc.CPUAccessFlags = CPU_ACCESS_NONE;
	texDesc.SampleCount = msaa;
	texDesc.MipLevels = mipLevels;
	texDesc.Width = width;
	texDesc.Height = height;

	ITexture* out;
	m_device->CreateTexture(texDesc, nullptr, &out);
	return out;
}

void DiligentContext::ResolveMSAA(RenderTarget* source, RenderTarget* dest)
{
	ResolveTextureSubresourceAttribs a;
	a.SrcTextureTransitionMode = a.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

	SOLAR_CORE_ASSERT(source->GetColorTargetCount() != dest->GetColorTargetCount());
	
	for (auto i = 0; i < source->GetColorRtvCount(); i++)
	{
		m_context->ResolveTextureSubresource(TO_TEXVIEW(source->colorRtvs[i])->GetTexture(), TO_TEXVIEW(dest->colorRtvs[i])->GetTexture(), a);
	}
}

void DiligentContext::SetRenderTarget(RenderTarget* texture, const bool autoTransition)
{
	m_activeTexture = texture;
	//if (autoTransition) TransitionState(texture, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_DEPTH_WRITE);

	m_context->SetRenderTargets(m_activeTexture->colorRtvs.size(), TO_TEXVIEW_ARR(m_activeTexture->colorRtvs.data()), TO_TEXVIEW(m_activeTexture->depthRtv), TRANSITION_MODE);
}

void DiligentContext::Clear(float* rgba, const float depth, const uint8_t stencil, const bool autoTransition)
{
	//if (autoTransition) TransitionState(m_activeTexture, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_DEPTH_WRITE);
	
	if (m_activeTexture->depthRtv)
		m_context->ClearDepthStencil(TO_TEXVIEW(m_activeTexture->depthRtv), CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, depth, stencil, TRANSITION_MODE);

	for (auto i = 0; i < m_activeTexture->colorRtvs.size(); i++)
		m_context->ClearRenderTarget(TO_TEXVIEW(m_activeTexture->colorRtvs[i]), rgba, TRANSITION_MODE);
}

void DiligentContext::BindMaterial(const Shared<Material>& material, const int subpass)
{
	m_context->SetPipelineState(material->shader->m_pipelineState);
	m_context->CommitShaderResources(material->m_mpb->m_resourceBinding, TRANSITION_MODE);
}

void DiligentContext::SubmitMesh(const Shared<Mesh>& mesh, const int subMesh)
{
	m_context->SetVertexBuffers(0, 1, &mesh->m_subMeshes[subMesh]->m_vertBuf, nullptr, TRANSITION_MODE, SET_VERTEX_BUFFERS_FLAG_RESET);
	m_context->SetIndexBuffer(mesh->m_subMeshes[subMesh]->m_idxBuf, 0, TRANSITION_MODE);

	DrawIndexedAttribs m;
	m.IndexType = VT_UINT32;
	m.NumIndices = mesh->m_subMeshes[subMesh]->m_idxCount;
	m.Flags = DRAW_FLAG_VERIFY_ALL;
	
	m_context->DrawIndexed(m);
}

void DiligentContext::Blit(RenderTarget* dest, const Shared<Material>& mat, int subpass)
{
	SetRenderTarget(dest);
	BindMaterial(mat, subpass);

	const DrawAttribs attribs{ 3, DRAW_FLAG_VERIFY_ALL };
	m_context->Draw(attribs);
}

void DiligentContext::Blit(Texture* src, RenderTarget* dest, const Shared<Material>& mat, int subpass)
{
	mat->GetProperties().SetTexture("_MainTex", src);
	Blit(dest, mat, subpass);
}

void DiligentContext::DispatchCompute(const glm::ivec3 groups, const Shared<Shader>& computeShader, MaterialPropertyBlock& mpb)
{
	m_context->SetPipelineState(computeShader->m_pipelineState);
	m_context->CommitShaderResources(mpb.m_resourceBinding.RawPtr(), TRANSITION_MODE);

	DispatchComputeAttribs attribs;
	attribs.ThreadGroupCountX = groups.x;
	attribs.ThreadGroupCountY = groups.y;
	attribs.ThreadGroupCountZ = groups.z;
	
	m_context->DispatchCompute(attribs);
}

void DiligentContext::UnbindVertexBuffers()
{
	m_context->SetVertexBuffers(0, 0, nullptr, nullptr, TRANSITION_MODE, SET_VERTEX_BUFFERS_FLAG_RESET);
	m_context->SetIndexBuffer(nullptr, 0, TRANSITION_MODE);
}

void DiligentContext::EndFrame()
{
	m_statsQuery->End(m_context, &m_pipelineStats, sizeof(m_pipelineStats));
	m_timerQuery->End(m_context, m_duration);
	
	m_context->Flush();
	m_context->WaitForIdle();
	m_context->FinishFrame();
}
