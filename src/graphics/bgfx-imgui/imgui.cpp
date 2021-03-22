/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
#include "pch.h"
#pragma warning(push, 1)
#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/ext.hpp"
#include "bx/math.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <dear-imgui/imgui.h>

#include "imgui.h"
#define USE_ENTRY 0

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"
#include "vs_imgui_texture.bin.h"
#include "fs_imgui_texture.bin.h"

#include "roboto_regular.ttf.h"
#include "robotomono_regular.ttf.h"
#include "icons_kenney.ttf.h"
#include "icons_font_awesome.ttf.h"
#include "core/assert.h"
#pragma warning(pop)

//TODO: Update ImGui version (docking branch)
//TODO: Rewrite this file to remove bloat, and to make detaching windows work

static OcornutImguiContext s_ctx;

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(vs_imgui_image),
	BGFX_EMBEDDED_SHADER(fs_imgui_image),
	BGFX_EMBEDDED_SHADER(vs_imgui_texture),
	BGFX_EMBEDDED_SHADER(fs_imgui_texture),

	BGFX_EMBEDDED_SHADER_END()
};

struct FontRangeMerge
{
	const void* data;
	size_t      size;
	ImWchar     ranges[3];
};

static FontRangeMerge s_fontRangeMerge[] =
{
	{ s_iconsKenneyTtf,      sizeof(s_iconsKenneyTtf),      { ICON_MIN_KI, ICON_MAX_KI, 0 } },
	{ s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), { ICON_MIN_FA, ICON_MAX_FA, 0 } },
};

inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout) && (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices));
}

struct ViewportData
{
	void* windowHandle;
	bgfx::FrameBufferHandle framebuffer;

	ViewportData() { windowHandle = nullptr; framebuffer = BGFX_INVALID_HANDLE; }
	~ViewportData() { windowHandle = nullptr; bgfx::destroy(framebuffer); }
};

static void createWindow(ImGuiViewport* viewport)
{
	auto data = IM_NEW(ViewportData)();
	viewport->RendererUserData = data;

	// PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*).
	// Some backend will leave PlatformHandleRaw NULL, in which case we assume PlatformHandle will contain the HWND.
	const auto windowHandle = viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle;
	SOLAR_CORE_ASSERT(hwnd != nullptr);

	data->windowHandle = windowHandle;
	data->framebuffer = bgfx::createFrameBuffer(windowHandle, viewport->Size.x, viewport->Size.y);
}

static void destroyWindow(ImGuiViewport* viewport)
{
	// The main viewport (owned by the application) will always have RendererUserData == NULL since we didn't create the data for it.
	if (auto* data = static_cast<ViewportData*>(viewport->RendererUserData))
	{
		IM_DELETE(data);
	}
	viewport->RendererUserData = nullptr;
}

static void setWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	const auto data = static_cast<ViewportData*>(viewport->RendererUserData);
	if (bgfx::isValid(data->framebuffer))
	{
		bgfx::destroy(data->framebuffer);
		data->framebuffer = bgfx::createFrameBuffer(data->windowHandle, size.x, size.y);
	}
}

static int s_id = 1;
static void renderWindow(ImGuiViewport* viewport, void*)
{	
	const auto data = static_cast<ViewportData*>(viewport->RendererUserData);
	if (!data) return;
	
	bgfx::resetView(s_id);
	bgfx::setViewFrameBuffer(s_id, data->framebuffer);
	
	bgfx::setViewClear(s_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0U, 1, 0);
	bgfx::touch(s_id);
	
	s_ctx.render(viewport, s_id);
	
	++s_id;
}

static void swapBuffers_noop(ImGuiViewport* viewport, void*) {}

void OcornutImguiContext::render(ImGuiViewport* viewport, bgfx::ViewId view) const
{
	const auto* drawData = viewport->DrawData;
	const auto x =      drawData->DisplayPos.x;
	const auto y =      drawData->DisplayPos.y;
	const auto width  = drawData->DisplaySize.x;
	const auto height = drawData->DisplaySize.y;
	
	bgfx::setViewName(view, "ImGui");
	bgfx::setViewMode(view, bgfx::ViewMode::Sequential);

	const bgfx::Caps* caps = bgfx::getCaps();
	
	//auto ortho = (caps->homogeneousDepth ? glm::orthoLH_NO<float> : glm::orthoLH_ZO<float>)(x, x + width, y + height, y, 0.0f, 1000.0f);
	float ortho[16];
	bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
	bgfx::setViewTransform(view, nullptr, ortho);
	bgfx::setViewRect(view, 0, 0, uint16_t(width), uint16_t(height) );

	// Render command lists
	for (auto ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
	{
		const ImDrawList* drawList = drawData->CmdLists[ii];
		const auto numVerts = static_cast<uint32_t>(drawList->VtxBuffer.size());
		const auto numIndices  = static_cast<uint32_t>(drawList->IdxBuffer.size());

		if (!checkAvailTransientBuffers(numVerts, m_vtxLayout, numIndices)) break;
		
		bgfx::TransientVertexBuffer tvb{};
		bgfx::allocTransientVertexBuffer(&tvb, numVerts, m_vtxLayout);
		
		bgfx::TransientIndexBuffer tib{};
		bgfx::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

		auto* verts = reinterpret_cast<ImDrawVert*>(tvb.data);
		memcpy(verts, drawList->VtxBuffer.begin(), numVerts * sizeof(ImDrawVert) );

		auto* indices = reinterpret_cast<ImDrawIdx*>(tib.data);
		memcpy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

		uint32_t offset = 0;
		for (const auto cmd : drawList->CmdBuffer)
		{
			if (cmd.UserCallback) 
			{
				if (cmd.UserCallback == ImDrawCallback_ResetRenderState); //bgfx::resetView(view);
				else cmd.UserCallback(drawList, &cmd);
			}
			else if (cmd.ElemCount != 0)
			{
				auto state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_EQUATION_SEPARATE(BGFX_STATE_BLEND_EQUATION_ADD, BGFX_STATE_BLEND_EQUATION_MAX);

				auto th = m_texHandle;
				auto program = m_defaultShader;

				if (cmd.TextureId != nullptr)
				{
					const union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd.TextureId };
					
					state |= (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags) != 0 ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) : BGFX_STATE_NONE;
					th = texture.s.handle;
					if (0 != texture.s.mip)
					{
						const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
						bgfx::setUniform(m_imgLodEnableUniform, lodEnabled);
						program = m_imageShader;
					}
				}
				else
				{
					state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					//const float lodEnabled[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
					//bgfx::setUniform(m_imgLodEnableUniform, lodEnabled);
				}

				const auto xx = glm::max(cmd.ClipRect.x - x, 0.0f);
				const auto yy = glm::max(cmd.ClipRect.y - y, 0.0f);
				
				bgfx::setScissor(xx, yy, uint16_t(glm::min(cmd.ClipRect.z - x, 65535.0f)-xx), uint16_t(glm::min(cmd.ClipRect.w - y, 65535.0f)-yy));

				bgfx::setState(state);
				bgfx::setTexture(0, m_texUniform, th);
				bgfx::setVertexBuffer(0, &tvb, 0, numVerts);
				bgfx::setIndexBuffer(&tib, offset, cmd.ElemCount);
				bgfx::submit(view, program, 0, BGFX_DISCARD_ALL);
			}

			offset += cmd.ElemCount;
		}
	}
}

void OcornutImguiContext::create(float _fontSize)
{
	//ImGui::SetAllocatorFunctions([](const size_t sz, void*) { return malloc(sz); }, [](void* ptr, void*) { free(ptr); }, nullptr);

	m_imguiCtx = ImGui::CreateContext();

	auto& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Renderer_CreateWindow = createWindow;
	platform_io.Renderer_DestroyWindow = destroyWindow;
	platform_io.Renderer_SetWindowSize = setWindowSize;
	platform_io.Renderer_RenderWindow = renderWindow;
	platform_io.Renderer_SwapBuffers = swapBuffers_noop;
	
	const auto type = bgfx::getRendererType();
	m_defaultShader = createProgram(createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_texture"), createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui"), true);

	m_imgLodEnableUniform = createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
	m_imageShader = createProgram(createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_texture"), createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_texture"), true);

	SOLAR_ASSERT_ALWAYS(bgfx::isValid(m_defaultShader));
	SOLAR_ASSERT_ALWAYS(bgfx::isValid(m_imageShader));

	m_vtxLayout
		.begin()
		.add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
		.end();

	m_texUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

	uint8_t* data;
	int32_t width;
	int32_t height;
	{
		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;
		config.MergeMode = false;
//			config.MergeGlyphCenterV = true;

		const auto ranges = io.Fonts->GetGlyphRangesCyrillic();

		// ReSharper disable CppCStyleCast
		m_fonts[0] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoRegularTtf,     sizeof(s_robotoRegularTtf),     _fontSize,      &config, ranges);
		m_fonts[1] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), _fontSize-3.0f, &config, ranges);
		// ReSharper restore CppCStyleCast

		config.MergeMode = true;
		config.DstFont   = m_fonts[0];

		for (auto& frm : s_fontRangeMerge)
		{
			io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(frm.data), frm.size, _fontSize-3.0f, &config, frm.ranges);
		}
	}

	io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
	m_texHandle = createTexture2D(static_cast<uint16_t>(glm::floor(width)), static_cast<uint16_t>(glm::floor(height)), false, 1, bgfx::TextureFormat::BGRA8, 0, bgfx::copy(data, width*height*4));
}

void OcornutImguiContext::destroy() const
{
	ImGui::DestroyPlatformWindows();
	ImGui::DestroyContext(m_imguiCtx);

	bgfx::destroy(m_texUniform);
	bgfx::destroy(m_texHandle);

	bgfx::destroy(m_imgLodEnableUniform);
	bgfx::destroy(m_imageShader);
	bgfx::destroy(m_defaultShader);
}

void imguiBeginFrame()
{
	s_id = 1;
}

void imguiRender(ImGuiViewport* data, bgfx::ViewId view)
{
	s_ctx.render(data, view);
}

void imguiCreate(float _fontSize)
{
	s_ctx.create(_fontSize);
}

void imguiDestroy()
{
	s_ctx.destroy();
}

////TODO: Wrap data in ImGuiInputState struct (or something similar)