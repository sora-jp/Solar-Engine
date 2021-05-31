#pragma once
#include "Texture.h"
#include "TextureUtils.h"

class CBRT : public RenderTarget_ 
{
	void* faces[6];

protected:
	void* const* GetColorRtvs() const override { return faces; }
	void* GetDepthRtv() const override { return nullptr; }

public:
	CBRT(ITexture* handle, int mip) 
	{
		TextureViewDesc d;
		d.ViewType = TEXTURE_VIEW_RENDER_TARGET;
		d.MostDetailedMip = mip;
		d.NumMipLevels = 1;
		for (size_t i = 0; i < 6; i++) 
		{
			d.FirstArraySlice = i;
			handle->CreateView(d, reinterpret_cast<ITextureView**>(&faces[i]));
		}
	}

	~CBRT() 
	{
		for (size_t i = 0; i < 6; i++) reinterpret_cast<ITextureView*>(faces[i])->Release();
	}
};

//inline void* LoadToCubemap(Texture& tex, void* handle) 
//{
//	ITexture* out;
//
//	TextureDesc desc;
//	desc.Type = RESOURCE_DIM_TEX_CUBE;
//	//desc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
//	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
//	desc.Format = TEX_FORMAT_RGBA32_FLOAT;
//	desc.Width  = 1024;
//	desc.Height = 1024;
//	desc.ArraySize = 6;
//	desc.MipLevels = 8;
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &out);
//	//GraphicsSubsystem::GetCurrentContext()->GetContext()->GenerateMips(result->texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
//
//	ITexture* tmpCubemap;
//	GraphicsSubsystem::GetContext()->GetDevice()->CreateTexture(desc, nullptr, &tmpCubemap);
//
//	static const Shared<Material> convertMat = Material::Create(ShaderCompiler::Compile("RectangularToCubemap.hlsl", "vert", "frag"));
//	static const Shared<Material> convolveMat = Material::Create(ShaderCompiler::Compile("SpecCubemapConvolution.hlsl", "vert", "frag"));
//
//	{
//		auto rt = CBRT(tmpCubemap, 0);
//
//		GraphicsSubsystem::GetContext()->SetRenderTarget(&rt);
//		convertMat->GetProperties().SetTexture("_MainTex", tex);
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
//		auto rtmip = CBRT(out, i);
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
//	tmpCubemap->Release();
//
//	return out;
//
//}