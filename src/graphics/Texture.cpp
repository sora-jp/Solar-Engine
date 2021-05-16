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
	if (desc.isRenderTarget && IsDSVFormat(desc.format)) return BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
	if (desc.isRenderTarget) return BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;

	return BIND_SHADER_RESOURCE;
}

TextureView::~TextureView()
{
	if (m_shouldRelease) static_cast<Diligent::ITextureView*>(m_texViewHandle)->Release();
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
	tex.Usage = USAGE_DEFAULT;
	tex.CPUAccessFlags = CPU_ACCESS_NONE;
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