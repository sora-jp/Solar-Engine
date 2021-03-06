//#pragma once
//#include "core/Common.h"
//#include <vector>
//#include <diligent/Common/interface/RefCntAutoPtr.hpp>
//#include <diligent/Graphics/GraphicsEngine/interface/Texture.h>
//
//struct TextureDescription
//{
//	Diligent::TEXTURE_FORMAT format;
//	uint32_t width, height;
//	uint32_t mipLevels;
//
//	TextureDescription(nullptr_t) : format(Diligent::TEX_FORMAT_UNKNOWN), width(-1), height(-1), mipLevels(-1) {  }
//	TextureDescription(const Diligent::TEXTURE_FORMAT fmt, const uint32_t width, const uint32_t height, const uint32_t mips = 1) : format(fmt), width(width), height(height), mipLevels(mips) {}
//
//	[[nodiscard]] bool Valid() const { return format != Diligent::TEX_FORMAT_UNKNOWN; }
//};
//
//struct RenderTargetDescription final : TextureDescription
//{
//	uint32_t numSamples;
//	RenderTargetDescription(nullptr_t) : TextureDescription(nullptr), numSamples(-1) {  }
//	RenderTargetDescription(const Diligent::TEXTURE_FORMAT fmt, const uint32_t width, const uint32_t height, const uint32_t mips = 1, const uint32_t numSamples = 1) : TextureDescription(fmt, width, height, mips), numSamples(numSamples) {}
//};
//
//class TextureBase
//{
//	friend class RenderTarget;
//	friend class MaterialPropertyBlock;
//	friend class PipelineContext;
//	
//	TextureBase(Diligent::ITexture* texture) : texture(texture) {}
//	Diligent::ITextureView* GetView(const Diligent::TEXTURE_VIEW_TYPE type) { return texture->GetDefaultView(type); }
//
//protected:
//	TextureBase() = default;
//	Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
//
//public:
//	void* ToImGui()
//	{
//		return GetView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
//	}
//};
//
//class Texture2D final : public TextureBase
//{
//	Texture2D() = default;
//	
//public:
//	static Shared<Texture2D> Create(const TextureDescription& desc);
//	static Shared<Texture2D> Load(const std::string& fileFormat);
//};
//
//class RenderTargetBase
//{
//public:
//	virtual ~RenderTargetBase() = default;
//private:
//	friend class DiligentContext;
//	
//	std::vector<Diligent::ITextureView*> m_colorTargets;
//	Diligent::ITextureView* m_depthTarget = nullptr;
//	
//	[[nodiscard]] virtual size_t GetColorTargetCount() { return m_colorTargets.size(); }
//	[[nodiscard]] virtual Diligent::ITextureView** GetColorTargets() { return m_colorTargets.data(); }
//	[[nodiscard]] virtual Diligent::ITextureView* GetDepthTarget() { return m_depthTarget; }
//
//protected:
//	RenderTargetBase() = default;
//	std::vector<Diligent::RefCntAutoPtr<Diligent::ITexture>> colorTextures;
//	Diligent::RefCntAutoPtr<Diligent::ITexture> depthTexture;
//
//	void Init(bool initColor, bool initDepth);
//};
//
//class RenderTarget final : public RenderTargetBase
//{
//	RenderTarget() : m_colorDesc(nullptr), m_depthDesc(nullptr) {}
//	RenderTargetDescription m_colorDesc, m_depthDesc;
//
//public:
//	static Shared<RenderTarget> Create(int colorTargetCount, const RenderTargetDescription& color, const RenderTargetDescription& depth);
//
//	[[nodiscard]] size_t ColorCount() const { return colorTextures.size(); }
//	[[nodiscard]] TextureBase Color(const int idx) { return colorTextures[idx].RawPtr(); }
//	[[nodiscard]] TextureBase Depth() { return depthTexture.RawPtr(); }
//	[[nodiscard]] uint32_t Width() const { return m_colorDesc.width; }
//	[[nodiscard]] uint32_t Height() const { return m_colorDesc.height; }
//};
//
//class CubemapRenderTarget final : public RenderTargetBase
//{
//	Diligent::ITextureView* m_colorTargets[6] {};
//
//	size_t GetColorTargetCount() override { return 6; }
//	Diligent::ITextureView** GetColorTargets() override { return m_colorTargets; }
//	Diligent::ITextureView* GetDepthTarget() override { return nullptr; }
//	
//public:
//	explicit CubemapRenderTarget(Diligent::ITexture* cubemap, const int mip = 0)
//	{
//		for (auto i = 0; i < 6; i++)
//		{
//			Diligent::TextureViewDesc view;
//			view.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
//			view.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
//			view.FirstArraySlice = i;
//			view.NumArraySlices = 1;
//			view.MostDetailedMip = mip;
//			view.NumMipLevels = 0;
//
//			cubemap->CreateView(view, &m_colorTargets[i]);
//		}
//	}
//
//	~CubemapRenderTarget() override
//	{
//		for (auto i = 0; i < 6; i++)
//		{
//			m_colorTargets[i]->Release();
//			m_colorTargets[i] = nullptr;
//		}
//	}
//};