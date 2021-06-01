#pragma once
#include "Texture.h"
#include "Material.h"
#include "Shader.h"

class CubemapRT final : RenderTarget_
{
public:
	CubemapRT(Diligent::ITexture* handle, const size_t mip = 0)
	{
		depthRtv = nullptr;
		
		colorRtvs.reserve(6);
		TextureViewDesc desc;
		desc.ViewType = TEXTURE_VIEW_RENDER_TARGET;
		desc.TextureDim = RESOURCE_DIM_TEX_2D;
		desc.MostDetailedMip = mip;
		desc.NumMipLevels = 1;
		desc.NumArraySlices = 1;
		
		for (size_t i = 0; i < 6; i++)
		{
			desc.FirstArraySlice = i;
			handle->CreateView(desc, reinterpret_cast<Diligent::ITextureView**>(&colorRtvs[i]));
		}
	}

	~CubemapRT()
	{
		for (size_t i = 0; i < 6; i++)
		{
			static_cast<Diligent::ITextureView*>(colorRtvs[i])->Release();
			colorRtvs[i] = nullptr;
		}
	}
};

inline void* EquirectToCubemap(const FullTextureDescription desc, Shared<Texture2D_> src)
{
	auto* target = static_cast<Diligent::ITexture*>(Create(TextureType::TexCube, desc, nullptr));

	static const auto convolveMat = Material::Create(ShaderCompiler::Compile("SpecCubemapConvolution.hlsl", "vert", "frag"));

	convolveMat->GetProperties().SetTexture("_MainTex", src);
	for (auto i = 0u; i < desc.mipLevels; i++)
	{
		SOLAR_CORE_INFO("Generating mip {}", i);

		const auto rough = static_cast<float>(i) / (desc.mipLevels - 1);
		convolveMat->GetProperties().Set("_Roughness", rough);

		auto rtmip = CubemapRT(target, i);
		GraphicsSubsystem::GetContext()->Blit(&rtmip, convolveMat);
	}

	return target;
}