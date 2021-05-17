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

BIND_FLAGS GetBindFlags(const _TextureDescription& desc)
{
	auto flags = BIND_SHADER_RESOURCE;
	
	if (desc.isRenderTarget && IsDSVFormat(desc.format)) flags |= BIND_DEPTH_STENCIL;
	else if (desc.isRenderTarget) flags |= BIND_RENDER_TARGET;

	return flags;
}

TextureView::TextureView(void* handle, const bool shouldRelease) : m_shouldRelease(shouldRelease), m_texViewHandle(handle)
{
	const auto vt = static_cast<Diligent::ITextureView*>(handle)->GetDesc().ViewType;
	m_isRenderTarget = vt == TEXTURE_VIEW_RENDER_TARGET || vt == TEXTURE_VIEW_DEPTH_STENCIL;
}

TextureView::~TextureView()
{
	if (m_shouldRelease) static_cast<Diligent::ITextureView*>(m_texViewHandle)->Release();
}

const TextureView& Texture::GetView(bool renderTarget)
{
	auto vt = TEXTURE_VIEW_SHADER_RESOURCE;
	if (renderTarget) vt = IsDSVFormat(m_description.format) ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET;
	
	return TextureView(static_cast<Diligent::ITexture*>(m_texHandle)->GetDefaultView(vt), false);
}

Shared<Texture> Texture::Create(const _TextureDescription& desc)
{
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
	
	ITexture* handle;
	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(tex, nullptr, &handle);
	
	return Shared<Texture>(new Texture(desc, handle));
}

Texture::~Texture()
{
	static_cast<Diligent::ITexture*>(m_texHandle)->Release();
}