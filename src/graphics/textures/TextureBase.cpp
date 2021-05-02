#include "pch.h"
#include "TextureBase.h"
#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"

using namespace Diligent;

TextureDesc ToDiligentDesc(const TextureDescription& desc)
{
	TextureDesc d;
	d.Format = desc.format;
	d.Width = desc.width;
	d.Height = desc.height;
	d.MipLevels = desc.mipLevels;
	d.Type = RESOURCE_DIM_TEX_2D;
	d.BindFlags = BIND_SHADER_RESOURCE;
	d.CPUAccessFlags = CPU_ACCESS_NONE;
	d.Usage = USAGE_DEFAULT;
	d.SampleCount = 1;
	
	return d;
}

TextureDesc ToDiligentDesc(const RenderTargetDescription& desc)
{
	auto d = ToDiligentDesc(static_cast<const TextureDescription&>(desc));
	
	if (   desc.format == TEX_FORMAT_D16_UNORM 
		|| desc.format == TEX_FORMAT_D32_FLOAT 
		|| desc.format == TEX_FORMAT_D24_UNORM_S8_UINT 
		|| desc.format == TEX_FORMAT_D32_FLOAT_S8X24_UINT) d.BindFlags |= BIND_DEPTH_STENCIL;
	else d.BindFlags |= BIND_RENDER_TARGET;
	
	d.SampleCount = desc.numSamples;

	return d;
}

Shared<Texture2D> Texture2D::Create(const TextureDescription& desc)
{
	auto tex = Shared<Texture2D>(new Texture2D());
	
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateTexture(ToDiligentDesc(desc), nullptr, &tex->texture);
	
	return tex;
}

void RenderTargetBase::Init(const bool initColor, const bool initDepth)
{
	if (initColor)
	{
		m_colorTargets.resize(colorTextures.size());
		std::transform(colorTextures.begin(), colorTextures.end(), m_colorTargets.begin(), [](RefCntAutoPtr<ITexture> t) { return t->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET); });
	}
	
	if (initDepth) m_depthTarget = depthTexture->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
}

Shared<RenderTarget> RenderTarget::Create(const int colorTargetCount, const RenderTargetDescription& color, const RenderTargetDescription& depth)
{
	auto rt = Shared<RenderTarget>(new RenderTarget());
	rt->m_colorDesc = color;
	rt->m_depthDesc = depth;

	if (color.Valid() && colorTargetCount > 0) {
		rt->colorTextures.resize(colorTargetCount);
		const auto c = ToDiligentDesc(color);

		for (auto i = 0; i < colorTargetCount; i++) GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateTexture(c, nullptr, &rt->colorTextures[i]);
	}
	
	if (depth.Valid()) GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateTexture(ToDiligentDesc(depth), nullptr, &rt->depthTexture);

	rt->Init(color.Valid() && colorTargetCount > 0, depth.Valid());
	return rt;
}