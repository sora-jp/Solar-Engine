#include "pch.h"
#include "DiligentInit.h"

#include "diligent/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "diligent/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "diligent/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "diligent/Platforms/Win32/interface/Win32NativeWindow.h"

#include "core/Log.h"
#include "diligent/DiligentWindow.h"
#include "RenderTexture.h"

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

void DiligentContext::TransitionState(RenderTexture* tex, const RESOURCE_STATE newColorState, const RESOURCE_STATE newDepthState)
{
	SOLAR_CORE_ASSERT(tex->IsValid());

	std::vector<StateTransitionDesc> transitions;

	//AppendTransitionDesc(transitions, tex->m_depthTarget, newDepthState);
	AppendTransitionDesc(transitions, tex->m_depthTarget->GetTexture(), newDepthState);

	for (auto i = 0; i < tex->m_numColorTargets; i++) 
	{
		//AppendTransitionDesc(transitions, tex->m_colorTargets[i], newColorState);
		AppendTransitionDesc(transitions, tex->m_colorTargets[i]->GetTexture(), newColorState);
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
			d3d11Info.DebugFlags = D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE;
				
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

	m_device->CreateBuffer(buf, nullptr, &m_constants);
	
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

void DiligentContext::SetRenderTarget(RenderTexture& texture, const bool autoTransition)
{
	m_activeTexture = texture;
	if (autoTransition) TransitionState(&texture, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_DEPTH_WRITE);

	m_context->SetRenderTargets(m_activeTexture.m_numColorTargets, &m_activeTexture.m_colorTargets[0], m_activeTexture.m_depthTarget, TRANSITION_MODE);
}

void DiligentContext::Clear(float* rgba, const float depth, const uint8_t stencil, const bool autoTransition)
{
	if (autoTransition) TransitionState(&m_activeTexture, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_DEPTH_WRITE);
	
	if (m_activeTexture.m_depthTarget)
		m_context->ClearDepthStencil(m_activeTexture.m_depthTarget, CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, depth, stencil, TRANSITION_MODE);

	for (auto i = 0; i < m_activeTexture.m_numColorTargets; i++)
		m_context->ClearRenderTarget(m_activeTexture.m_colorTargets[i], rgba, TRANSITION_MODE);
}

void DiligentContext::BindMaterial(const Shared<Material>& material, const int subpass)
{
	m_context->SetPipelineState(material->shader->m_passes[subpass]->m_pipelineState);
	m_context->CommitShaderResources(material->m_bindings[subpass], TRANSITION_MODE);
}

void DiligentContext::SubmitMesh(const Shared<Mesh>& mesh)
{
	m_context->SetVertexBuffers(0, 1, &mesh->m_vertBuf, nullptr, TRANSITION_MODE, SET_VERTEX_BUFFERS_FLAG_RESET);
	m_context->SetIndexBuffer(mesh->m_idxBuf, 0, TRANSITION_MODE);

	DrawIndexedAttribs m;
	m.IndexType = VT_UINT32;
	m.NumIndices = mesh->m_idxCount;
	m.Flags = DRAW_FLAG_VERIFY_ALL;
	
	m_context->DrawIndexed(m);
}

void DiligentContext::SetModelMatrix(const glm::mat4 matrix)
{
	auto map = MapConstants();
	map->model = glm::transpose(matrix);
}

void DiligentContext::EndFrame()
{
	m_statsQuery->End(m_context, &m_pipelineStats, sizeof(m_pipelineStats));
	m_timerQuery->End(m_context, m_duration);
	
	m_context->FinishFrame();
}
