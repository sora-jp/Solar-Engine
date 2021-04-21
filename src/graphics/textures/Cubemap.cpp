#include "pch.h"
#include "Cubemap.h"
#include "core/Log.h"
#include "freeimage/FreeImage.h"
#include "GraphicsSubsystem.h"

using namespace Diligent;

FIBITMAP* LoadFreeImgBitmap(const char* path, const int flags) {
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

Shared<Cubemap> Cubemap::Load(const std::string& file)
{
	auto result = MakeShared<Cubemap>();

	auto* bitmap = LoadFreeImgBitmap(file.c_str(), 0);
	if (bitmap == nullptr)
	{
		SOLAR_CORE_ERROR("Loading cubemap {} failed", file.c_str());
		return nullptr;
	}

	const auto colorType = FreeImage_GetColorType(bitmap);
	const auto pixType = FreeImage_GetImageType(bitmap);

	auto fmt = TEX_FORMAT_UNKNOWN;
	SOLAR_CORE_TRACE("Loading image {} (color type: {}, pixel type: {})", file, colorType, pixType);
	
	if (colorType == FIC_RGB)
	{
		if (pixType == FIT_RGBF || pixType == FIT_FLOAT) fmt = TEX_FORMAT_RGB32_FLOAT;
		else if (pixType == FIT_RGB16 || pixType == FIT_UINT16) 
		{
			fmt = TEX_FORMAT_RGBA16_UNORM;
			SOLAR_CORE_WARN("Diligent doesn't support RGB16 textures, converting to RGBA16");
			bitmap = FreeImage_ConvertToRGBA16(bitmap);
		}
		else if (pixType == FIT_BITMAP)
		{
			fmt = TEX_FORMAT_RGBA32_FLOAT;
			bitmap = FreeImage_ConvertToRGBAF(bitmap);
		}
	}
	else if (colorType == FIC_RGBALPHA)
	{
		if (pixType == FIT_RGBAF) fmt = TEX_FORMAT_RGBA32_FLOAT;
		else if (pixType == FIT_RGBA16) fmt = TEX_FORMAT_RGBA16_UNORM;
	}
	else
	{
		SOLAR_CORE_ERROR("Unkown color type: {}", colorType);
	}
	
	TextureDesc desc;
	desc.Type = RESOURCE_DIM_TEX_2D;
	desc.Usage = USAGE_DEFAULT;
	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
	desc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
	desc.SampleCount = 1;
	desc.MipLevels = 0;
	
	desc.Format = fmt;
	desc.Width = FreeImage_GetWidth(bitmap);
	desc.Height = FreeImage_GetHeight(bitmap);

	TextureSubResData subResource;
	subResource.Stride = FreeImage_GetPitch(bitmap);
	subResource.pData = FreeImage_GetBits(bitmap);

	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateTexture(desc, nullptr, &result->texture);
	GraphicsSubsystem::GetCurrentContext()->GetContext()->UpdateTexture(result->texture, 0, 0, Box(0, desc.Width, 0, desc.Height), subResource, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	GraphicsSubsystem::GetCurrentContext()->GetContext()->GenerateMips(result->texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

	FreeImage_Unload(bitmap);

	return result;
}
