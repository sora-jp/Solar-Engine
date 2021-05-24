#include "pch.h"
#include "Texture.h"

#include <diligent/Graphics/GraphicsEngine/interface/Texture.h>
#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"

bool IsDSVFormat(const TextureFormat fmt)
{
	switch (fmt)
	{
		case TextureFormat::D16:
		case TextureFormat::D32:
		case TextureFormat::D32_S8:
		case TextureFormat::D24_S8:
			return true;
		
		default: return false;
	}
}

TEXTURE_FORMAT MapFormat(const TextureFormat fmt)
{
	switch (fmt)
	{
		case TextureFormat::RGBA32:		return TEX_FORMAT_RGBA32_FLOAT;
		case TextureFormat::RGB32:		return TEX_FORMAT_RGB32_FLOAT;
		case TextureFormat::RG32:		return TEX_FORMAT_RG32_FLOAT;
		case TextureFormat::RGBA8:		return TEX_FORMAT_RGBA8_UNORM;
		case TextureFormat::RGBA8_SRGB: return TEX_FORMAT_RGBA8_UNORM_SRGB;
		case TextureFormat::D16:		return TEX_FORMAT_D16_UNORM;
		case TextureFormat::D32:		return TEX_FORMAT_D32_FLOAT;
		case TextureFormat::D32_S8:		return TEX_FORMAT_D32_FLOAT_S8X24_UINT;
		case TextureFormat::D24_S8:		return TEX_FORMAT_D24_UNORM_S8_UINT;
	}
	
	SOLAR_CORE_DIE("Invalid texture format");
}

RESOURCE_DIMENSION MapType(const TextureType type)
{
	switch (type)
	{
		case TextureType::Tex2D:		return RESOURCE_DIM_TEX_2D;
		case TextureType::Tex2DArray:	return RESOURCE_DIM_TEX_2D_ARRAY;
		case TextureType::Tex3D:		return RESOURCE_DIM_TEX_3D;
		case TextureType::TexCube:		return RESOURCE_DIM_TEX_CUBE;
	}

	SOLAR_CORE_DIE("Invalid texture type");
}

BIND_FLAGS GetBindFlags(const FullTextureDescription& desc)
{
	auto flags = BIND_SHADER_RESOURCE;
	
	if (desc.isRenderTarget && IsDSVFormat(desc.format)) flags |= BIND_DEPTH_STENCIL;
	else if (desc.isRenderTarget) flags |= BIND_RENDER_TARGET;

	return flags;
}

TextureView::TextureView(void* handle)
{
	m_texViewHandle = handle;
	static_cast<Diligent::ITextureView*>(m_texViewHandle)->AddRef();
}

void TextureView::Release()
{
	if (m_texViewHandle != nullptr) static_cast<Diligent::ITextureView*>(m_texViewHandle)->Release();
	m_texViewHandle = nullptr;
}

void Texture::Create(const FullTextureDescription& desc)
{
	description = desc;
	
	TextureDesc tex;
	tex.Type = MapType(desc.type);
	tex.Width = desc.width;
	tex.Height = desc.height;
	tex.Depth = desc.depth;
	tex.BindFlags = GetBindFlags(desc);
	tex.Format = MapFormat(desc.format);
	tex.Usage = desc.cpuReadWriteEnabled ? USAGE_UNIFIED : USAGE_DEFAULT;
	tex.CPUAccessFlags = desc.cpuReadWriteEnabled ? CPU_ACCESS_READ | CPU_ACCESS_WRITE : CPU_ACCESS_NONE;
	tex.MipLevels = desc.mipLevels;
	tex.SampleCount = 1;

	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(tex, nullptr, reinterpret_cast<ITexture**>(&texHandle));

	srv = TextureView(static_cast<Diligent::ITexture*>(texHandle)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
}

Texture::~Texture()
{
	static_cast<Diligent::ITexture*>(texHandle)->Release();
}

static FullTextureDescription Convert(const RenderTextureDesc d, const bool isDepth) { return { TextureType::Tex2D, isDepth ? d.depthBufferFormat : d.colorBufferFormat, d.width, d.height, 1, 1, false, true }; }
RenderTextureAttachment::RenderTextureAttachment(const RenderTextureDesc& desc, const bool isDepth)
{
	Create(desc, isDepth);
}

void RenderTextureAttachment::Create(const RenderTextureDesc& desc, bool isDepth)
{
	Texture::Create(Convert(desc, isDepth));
	m_rtv = TextureView(static_cast<Diligent::ITexture*>(texHandle)->GetDefaultView(isDepth ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET));
}

RenderTexture::RenderTexture(const RenderTextureDesc& desc, const size_t numColor, const bool depth) noexcept
{
	m_description = desc;

	m_attachments = std::vector(numColor + 1, RenderTextureAttachment());
	m_cachedRtvs = std::vector(numColor + 1, TextureView());
	
	for (size_t i = 0; i < numColor; i++)
	{
		m_attachments[i + 1].Create(desc, false);
		m_cachedRtvs[i + 1] = m_attachments[i].GetRenderTargetView();
	}

	if (depth)
	{
		m_attachments[0].Create(desc, true);
		m_cachedRtvs[0] = m_attachments[0].GetRenderTargetView();
	}
}

Shared<RenderTexture> RenderTexture::Create(const RenderTextureDesc& desc, const size_t numColor, const bool depth)
{
	return Shared<RenderTexture>(new RenderTexture(desc, numColor, depth));
}
