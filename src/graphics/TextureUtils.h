#pragma once

#include <diligent/Graphics/GraphicsEngine/interface/Texture.h>
#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"
#include "Texture.h"
#include <freeimage/FreeImage.h>

inline bool IsDSVFormat(const TextureFormat fmt)
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

inline TEXTURE_FORMAT MapFormat(const TextureFormat fmt)
{
	switch (fmt)
	{
		case TextureFormat::RGBA16:		return TEX_FORMAT_RGBA16_UNORM;
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

inline RESOURCE_DIMENSION MapType(const TextureType type)
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

inline BIND_FLAGS GetBindFlags(const FullTextureDescription& desc)
{
	auto flags = BIND_SHADER_RESOURCE;

	if (desc.isRenderTarget && IsDSVFormat(desc.format)) flags |= BIND_DEPTH_STENCIL;
	else if (desc.isRenderTarget) flags |= BIND_RENDER_TARGET | BIND_UNORDERED_ACCESS;

	if (desc.mipLevels != 1 && desc.generateMips) flags |= BIND_RENDER_TARGET;

	return flags;
}

inline void* Create(const TextureType type, const FullTextureDescription& desc, const TextureSubResData* data)
{
	ITexture* out = nullptr;

	TextureDesc tex;
	tex.Type = MapType(type);
	tex.Width = desc.width;
	tex.Height = desc.height;
	tex.Depth = desc.depth;
	tex.BindFlags = GetBindFlags(desc);
	tex.Format = MapFormat(desc.format);
	tex.Usage = desc.cpuReadWriteEnabled ? USAGE_UNIFIED : USAGE_DEFAULT;
	tex.CPUAccessFlags = desc.cpuReadWriteEnabled ? CPU_ACCESS_READ | CPU_ACCESS_WRITE : CPU_ACCESS_NONE;
	tex.MipLevels = desc.mipLevels;
	tex.MiscFlags = desc.mipLevels != 1 && desc.generateMips ? MISC_TEXTURE_FLAG_GENERATE_MIPS : MISC_TEXTURE_FLAG_NONE;
	tex.SampleCount = 1;

	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(tex, nullptr, &out);

	if (data)
	{
		GraphicsSubsystem::GetContext()->GetContext()->UpdateTexture(out, 0, 0, { 0, desc.width, 0, desc.height }, *data, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	if (desc.generateMips)
	{
		GraphicsSubsystem::GetContext()->GetContext()->GenerateMips(out->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
	}
	
	return out;
}

inline FIBITMAP* LoadFreeImgBitmap(const char* path, const int flags) {
	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	auto fif = FreeImage_GetFileType(path, 0);
	if (fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(path);
	}
	// check that the plugin has reading capabilities ...
	if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif)) {
		// ok, let's load the file
		// unless a bad file format, we are done !
		return FreeImage_Load(fif, path, flags);
	}
	return nullptr;
}

inline void* Load(const std::string& path, FullTextureDescription& desc, uint32_t downsampleFactor = 0)
{
	auto* bitmap = LoadFreeImgBitmap(path.c_str(), 0);
	if (bitmap == nullptr)
	{
		SOLAR_CORE_ERROR("Loading texture {} failed", path.c_str());
		return nullptr;
	}

	const auto colorType = FreeImage_GetColorType(bitmap);
	const auto pixType = FreeImage_GetImageType(bitmap);

	auto fmt = TextureFormat::Unknown;
	SOLAR_CORE_TRACE("Loading image {} (color type: {}, pixel type: {})", path, colorType, pixType);

	if (colorType == FIC_RGB)
	{
		if (pixType == FIT_RGBF || pixType == FIT_FLOAT) fmt = TextureFormat::RGB32;
		else if (pixType == FIT_RGB16 || pixType == FIT_UINT16)
		{
			fmt = TextureFormat::RGBA16;
			SOLAR_CORE_WARN("Diligent doesn't support RGB16 textures, converting to RGBA16");
			bitmap = FreeImage_ConvertToRGBA16(bitmap);
		}
		else if (pixType == FIT_BITMAP)
		{
			fmt = TextureFormat::RGBA32;
			bitmap = FreeImage_ConvertToRGBAF(bitmap);
		}
	}
	else if (colorType == FIC_RGBALPHA)
	{
		if (pixType == FIT_RGBAF) fmt = TextureFormat::RGBA32;
		else if (pixType == FIT_RGBA16) fmt = TextureFormat::RGBA16;
		else if (pixType == FIT_BITMAP)
		{
			fmt = TextureFormat::RGBA32;
			bitmap = FreeImage_ConvertToRGBAF(bitmap);
		}
	}
	
	if (fmt == TextureFormat::Unknown)
	{
		SOLAR_CORE_ERROR("Unkown color type: {}", colorType);
		return nullptr;
	}

	if (downsampleFactor > 0)
	{
		bitmap = FreeImage_Rescale(bitmap, FreeImage_GetWidth(bitmap) >> downsampleFactor, FreeImage_GetHeight(bitmap) >> downsampleFactor, FILTER_BICUBIC);
	}

	TextureSubResData subResource;
	subResource.Stride = FreeImage_GetPitch(bitmap);
	subResource.pData = FreeImage_GetBits(bitmap);
	subResource.pSrcBuffer = nullptr;

	desc.format = fmt;

	desc.width = FreeImage_GetWidth(bitmap);
	desc.height = FreeImage_GetHeight(bitmap);

	desc.generateMips = true;
	desc.mipLevels = 0;

	const auto res = Create(TextureType::Tex2D, desc, &subResource);

	FreeImage_Unload(bitmap);
	return res;
}