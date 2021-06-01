//#include "pch.h"
//#include "TextureBase.h"
//
//#include <freeimage/FreeImage.h>
//
//#include "GraphicsSubsystem.h"
//#include "diligent/DiligentInit.h"
//
//using namespace Diligent;
//
//TextureDesc ToDiligentDesc(const TextureDescription& desc)
//{
//	TextureDesc d;
//	d.Format = desc.format;
//	d.Width = desc.width;
//	d.Height = desc.height;
//	d.MipLevels = desc.mipLevels;
//	d.Type = RESOURCE_DIM_TEX_2D;
//	d.BindFlags = BIND_SHADER_RESOURCE;
//	d.CPUAccessFlags = CPU_ACCESS_NONE;
//	d.Usage = USAGE_DEFAULT;
//	d.SampleCount = 1;
//	
//	return d;
//}
//
//TextureDesc ToDiligentDesc(const RenderTargetDescription& desc)
//{
//	auto d = ToDiligentDesc(static_cast<const TextureDescription&>(desc));
//	
//	if (   desc.format == TEX_FORMAT_D16_UNORM 
//		|| desc.format == TEX_FORMAT_D32_FLOAT 
//		|| desc.format == TEX_FORMAT_D24_UNORM_S8_UINT 
//		|| desc.format == TEX_FORMAT_D32_FLOAT_S8X24_UINT) d.BindFlags |= BIND_DEPTH_STENCIL;
//	else d.BindFlags |= BIND_RENDER_TARGET | BIND_UNORDERED_ACCESS;
//	
//	d.SampleCount = desc.numSamples;
//
//	return d;
//}
//
//Shared<Texture2D> Texture2D::Create(const TextureDescription& desc)
//{
//	auto tex = Shared<Texture2D>(new Texture2D());
//	
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(ToDiligentDesc(desc), nullptr, &tex->texture);
//	
//	return tex;
//}
//
//static FIBITMAP* LoadFreeImgBitmap(const char* path, const int flags) {
//	// check the file signature and deduce its format
//	// (the second argument is currently not used by FreeImage)
//	auto fif = FreeImage_GetFileType(path, 0);
//	if (fif == FIF_UNKNOWN) {
//		// no signature ?
//		// try to guess the file format from the file extension
//		fif = FreeImage_GetFIFFromFilename(path);
//	}
//	// check that the plugin has reading capabilities ...
//	if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif)) {
//		// ok, let's load the file
//		// unless a bad file format, we are done !
//		return FreeImage_Load(fif, path, flags);
//	}
//	return nullptr;
//}
//
//Shared<Texture2D> Texture2D::Load(const std::string& fileFormat)
//{
//	auto result = Shared<Texture2D>(new Texture2D());
//	
//	auto* bitmap = LoadFreeImgBitmap(fileFormat.c_str(), 0);
//	if (bitmap == nullptr)
//	{
//		SOLAR_CORE_WARN("Failed to load texture");
//		return nullptr;
//	}
//
//	const auto colorType = FreeImage_GetColorType(bitmap);
//	const auto pixType = FreeImage_GetImageType(bitmap);
//
//	auto fmt = TEX_FORMAT_UNKNOWN;
//	//SOLAR_CORE_TRACE("Loading image (color type: {}, pixel type: {})", colorType, pixType);
//
//	if (colorType == FIC_RGB)
//	{
//		if (pixType == FIT_RGBF || pixType == FIT_FLOAT) fmt = TEX_FORMAT_RGB32_FLOAT;
//		else if (pixType == FIT_RGB16 || pixType == FIT_UINT16)
//		{
//			fmt = TEX_FORMAT_RGBA16_UNORM;
//			SOLAR_CORE_WARN("Diligent doesn't support RGB16 textures, converting to RGBA16");
//			bitmap = FreeImage_ConvertToRGBA16(bitmap);
//		}
//		else if (pixType == FIT_BITMAP)
//		{
//			fmt = TEX_FORMAT_RGBA32_FLOAT;
//			bitmap = FreeImage_ConvertToRGBAF(bitmap);
//		}
//	}
//	else if (colorType == FIC_RGBALPHA)
//	{
//		if (pixType == FIT_RGBAF) fmt = TEX_FORMAT_RGBA32_FLOAT;
//		else if (pixType == FIT_RGBA16) fmt = TEX_FORMAT_RGBA16_UNORM;
//		else if (pixType == FIT_BITMAP)
//		{
//			fmt = TEX_FORMAT_RGBA32_FLOAT;
//			bitmap = FreeImage_ConvertToRGBAF(bitmap);
//		}
//	}
//	else
//	{
//		SOLAR_CORE_ERROR("Unkown color type: {}", colorType);
//	}
//
//	TextureDesc desc;
//	desc.Type = RESOURCE_DIM_TEX_2D;
//	desc.Usage = USAGE_DEFAULT;
//	desc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
//	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
//	desc.SampleCount = 1;
//	desc.MipLevels = 0;
//
//	desc.Format = fmt;
//	desc.Width = FreeImage_GetWidth(bitmap);
//	desc.Height = FreeImage_GetHeight(bitmap);
//
//	TextureSubResData subResource;
//	subResource.Stride = FreeImage_GetPitch(bitmap);
//	subResource.pData = FreeImage_GetBits(bitmap);
//	
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &result->texture);
//	GraphicsSubsystem::GetContext()->GetContext()->UpdateTexture(result->texture, 0, 0, Box(0, desc.Width, 0, desc.Height), subResource, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
//	GraphicsSubsystem::GetContext()->GetContext()->GenerateMips(result->texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
//
//	return result;
//}
//
//void RenderTargetBase::Init(const bool initColor, const bool initDepth)
//{
//	if (initColor)
//	{
//		m_colorTargets.resize(colorTextures.size());
//		std::transform(colorTextures.begin(), colorTextures.end(), m_colorTargets.begin(), [](RefCntAutoPtr<ITexture> t) { return t->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET); });
//	}
//	
//	if (initDepth) m_depthTarget = depthTexture->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
//}
//
//Shared<RenderTarget> RenderTarget::Create(const int colorTargetCount, const RenderTargetDescription& color, const RenderTargetDescription& depth)
//{
//	auto rt = Shared<RenderTarget>(new RenderTarget());
//	rt->m_colorDesc = color;
//	rt->m_depthDesc = depth;
//
//	if (color.Valid() && colorTargetCount > 0) {
//		rt->colorTextures.resize(colorTargetCount);
//		const auto c = ToDiligentDesc(color);
//
//		for (auto i = 0; i < colorTargetCount; i++) GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(c, nullptr, &rt->colorTextures[i]);
//	}
//	
//	if (depth.Valid()) GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(ToDiligentDesc(depth), nullptr, &rt->depthTexture);
//
//	rt->Init(color.Valid() && colorTargetCount > 0, depth.Valid());
//	return rt;
//}