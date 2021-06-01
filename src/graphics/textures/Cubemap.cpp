#include "pch.h"
//#include "Cubemap.h"
//#include "core/Log.h"
//#include "freeimage/FreeImage.h"
//#include "GraphicsSubsystem.h"
//#include "diligent/DiligentInit.h"
//
//using namespace Diligent;
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
//class RawRenderTarget final : public RenderTargetBase
//{
//	ITextureView** m_color;
//	size_t m_colorCount;
//	ITextureView* m_depth;
//	
//public:
//	RawRenderTarget(ITextureView** color, size_t colorCount, ITextureView* depth) : m_color(color), m_colorCount(colorCount), m_depth(depth) {}
//	
//	[[nodiscard]] size_t GetColorTargetCount() override { return m_colorCount; }
//	[[nodiscard]] Diligent::ITextureView** GetColorTargets() override { return m_color; }
//	[[nodiscard]] Diligent::ITextureView* GetDepthTarget() override { return m_depth; }
//};
//
//class RawTexture final : public TextureBase
//{
//public:
//	RawTexture(ITexture* tex)
//	{
//		texture = tex;
//	}
//};
//
//Shared<Cubemap> Cubemap::Load(const std::string& file)
//{
//	auto result = MakeShared<Cubemap>();
//
//	auto* bitmap = LoadFreeImgBitmap(file.c_str(), 0);
//	if (bitmap == nullptr)
//	{
//		SOLAR_CORE_ERROR("Loading cubemap {} failed", file.c_str());
//		return nullptr;
//	}
//
//	const auto colorType = FreeImage_GetColorType(bitmap);
//	const auto pixType = FreeImage_GetImageType(bitmap);
//
//	auto fmt = TEX_FORMAT_UNKNOWN;
//	SOLAR_CORE_TRACE("Loading image {} (color type: {}, pixel type: {})", file, colorType, pixType);
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
//	}
//	else
//	{
//		SOLAR_CORE_ERROR("Unkown color type: {}", colorType);
//	}
//	
//	TextureDesc desc;
//	desc.Type = RESOURCE_DIM_TEX_2D;
//	desc.Usage = USAGE_DEFAULT;
//	desc.BindFlags = BIND_SHADER_RESOURCE;
//	desc.SampleCount = 1;
//	desc.MipLevels = 1;
//	
//	desc.Format = fmt;
//	desc.Width = FreeImage_GetWidth(bitmap);
//	desc.Height = FreeImage_GetHeight(bitmap);
//
//	TextureSubResData subResource;
//	subResource.Stride = FreeImage_GetPitch(bitmap);
//	subResource.pData = FreeImage_GetBits(bitmap);
//
//	TextureData data;
//	data.NumSubresources = 1;
//	data.pSubResources = &subResource;
//
//	ITexture* tempTexture;
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, &data, &tempTexture);
//
//	desc.Type = RESOURCE_DIM_TEX_CUBE;
//	//desc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
//	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
//	desc.Format = TEX_FORMAT_RGBA32_FLOAT;
//	desc.Width  = 1024;
//	desc.Height = 1024;
//	desc.ArraySize = 6;
//	desc.MipLevels = 8;
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &result->texture);
//	//GraphicsSubsystem::GetCurrentContext()->GetContext()->GenerateMips(result->texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
//
//	ITexture* tmpCubemap;
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &tmpCubemap);
//
//	static const Shared<Material> convertMat = Material::Create(ShaderCompiler::Compile("RectangularToCubemap.hlsl", "vert", "frag"));
//	static const Shared<Material> convolveMat = Material::Create(ShaderCompiler::Compile("SpecCubemapConvolution.hlsl", "vert", "frag"));
//
//	{
//		auto rt = CubemapRenderTarget(tmpCubemap);
//
//		GraphicsSubsystem::GetContext()->SetRenderTarget(&rt);
//		convertMat->GetProperties().SetTexture("_MainTex", RawTexture(tempTexture));
//
//		GraphicsSubsystem::GetContext()->RenderFullscreenQuad(convertMat);
//	}
//
//
//	for (auto i = 0; i < 8; i++)
//	{
//		SOLAR_CORE_INFO("Generating mip {}", i);
//		
//		const auto rough = static_cast<float>(i) / 7;
//		auto rtmip = CubemapRenderTarget(result->texture, i);
//		
//		convolveMat->GetProperties().SetTexture("_MainTex", RawTexture(tmpCubemap));
//		convolveMat->GetProperties().Set("_Roughness", rough);
//
//		GraphicsSubsystem::GetContext()->SetRenderTarget(&rtmip);
//		GraphicsSubsystem::GetContext()->RenderFullscreenQuad(convolveMat);
//
//		GraphicsSubsystem::GetContext()->GetContext()->WaitForIdle();
//	}
//
//	tempTexture->Release();
//	tmpCubemap->Release();
//	
//	FreeImage_Unload(bitmap);
//
//	return result;
//}
//
//Shared<Cubemap> Cubemap::ConvolveDiffuse(const Shared<Cubemap>& other)
//{
//	auto result = MakeShared<Cubemap>();
//
//	TextureDesc desc;
//	desc.Usage = USAGE_DEFAULT;
//	desc.Type = RESOURCE_DIM_TEX_CUBE;
//	//desc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
//	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
//	desc.Format = TEX_FORMAT_RGBA32_FLOAT;
//	desc.Width = 128;
//	desc.Height = 128;
//	desc.ArraySize = 6;
//	desc.MipLevels = 1;
//	desc.SampleCount = 1;
//	
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &result->texture);
//
//	auto rt = CubemapRenderTarget(result->texture);
//
//	static const Shared<Material> convolveMat = Material::Create(ShaderCompiler::Compile("DiffuseCubemapConvolution.hlsl", "vert", "frag"));
//
//	GraphicsSubsystem::GetContext()->SetRenderTarget(&rt);
//	convolveMat->GetProperties().SetTexture("_MainTex", *other);
//
//	GraphicsSubsystem::GetContext()->RenderFullscreenQuad(convolveMat);
//	
//	return result;
//}
