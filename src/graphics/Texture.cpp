#include "pch.h"
#include "Texture.h"
#include "TextureUtils.h"


Texture::Texture(TextureType type, const FullTextureDescription& desc) noexcept
{
	description = desc;

	texHandle = Create(type, desc, nullptr);
	srv = static_cast<Diligent::ITexture*>(texHandle)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

Texture::Texture(TextureType type, const std::string& path) noexcept
{
	description = FullTextureDescription();
	texHandle = Load(path, description);
	srv = static_cast<Diligent::ITexture*>(texHandle)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

Texture::~Texture()
{
	static_cast<Diligent::ITexture*>(texHandle)->Release();
}

static FullTextureDescription Convert(const RenderTextureDesc d, const bool isDepth) { return { isDepth ? d.depthBufferFormat : d.colorBufferFormat, d.width, d.height, 1, 1, false, true }; }
RenderTextureAttachment::RenderTextureAttachment(const RenderTextureDesc& desc, const bool isDepth) : Texture(TextureType::Tex2D, Convert(desc, isDepth))
{
	m_rtv = static_cast<Diligent::ITexture*>(texHandle)->GetDefaultView(isDepth ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET);
}

RenderTexture::RenderTexture(const RenderTextureDesc& desc, const size_t numColor, const bool depth) noexcept
{
	m_description = desc;

	m_attachments.reserve(numColor + 1);
	m_cachedRtvs.reserve(numColor + 1);
	
	for (size_t i = 0; i < numColor; i++)
	{
		m_attachments[i + 1] = new RenderTextureAttachment(desc, false);
		m_cachedRtvs[i + 1] = m_attachments[i + 1]->GetRenderTargetView();
	}

	if (depth)
	{
		m_attachments[0] = new RenderTextureAttachment(desc, true);
		m_cachedRtvs[0] = m_attachments[0]->GetRenderTargetView();
	}
	else
	{
		m_attachments[0] = nullptr;
		m_cachedRtvs[0] = nullptr;
	}
}

Shared<RenderTexture> RenderTexture::Create(const RenderTextureDesc& desc, const size_t numColor, const bool depth)
{
	return Shared<RenderTexture>(new RenderTexture(desc, numColor, depth));
}

RenderTexture::~RenderTexture()
{
	for (size_t i = 0; i < m_attachments.size(); i++) delete m_attachments[i];
}
