#pragma once
#include "core/Common.h"
#include <cstdint>

enum class TextureFormat
{
	Unknown, RGBA32, RGB32, RG32, RGBA8, RGBA8_SRGB, D16, D32, D32_S8, D24_S8
};

enum class TextureType
{
	Unknown, Tex2D, Tex2DArray, Tex3D, TexCube 
};

struct FullTextureDescription
{
	TextureType type = TextureType::Unknown;
	TextureFormat format = TextureFormat::Unknown;
	
	uint32_t width = 0, height = 0;
	union { uint32_t arraySize = 1; uint32_t depth; };
	
	uint32_t mipLevels = 1;
	bool cpuReadWriteEnabled = false;
	bool isRenderTarget = false;
};

class TextureView
{
	friend class Texture;
	friend class RenderTexture;
	friend class RenderTextureAttachment;
	friend class DiligentContext;
	
	void* m_texViewHandle;
	
	explicit TextureView(void* handle = nullptr);

protected:
	void Release();
};

class Texture
{
	friend class DiligentContext;

protected:
	FullTextureDescription description;
	TextureView srv;
	void* texHandle;

	Texture() noexcept : texHandle(nullptr) {}
	explicit Texture(const FullTextureDescription& desc) noexcept : texHandle(nullptr) { Create(desc); }
	void Create(const FullTextureDescription& desc);
	
	const TextureView& GetResourceView() const { return srv; }

public:
	Texture(const Texture& other) noexcept = delete;
	Texture(Texture&& other) noexcept = delete;
	Texture& operator=(const Texture&& other) noexcept = delete;
	Texture& operator=(const Texture& other) noexcept = delete;
	
	~Texture();
	
	[[nodiscard]] uint32_t Width() const { return description.width; }
	[[nodiscard]] uint32_t Height() const { return description.height; }
};

struct RenderTextureDesc
{
	uint32_t width = 0, height = 0;
	TextureFormat colorBufferFormat = TextureFormat::RGBA32;
	TextureFormat depthBufferFormat = TextureFormat::D24_S8;
};

class RenderTextureAttachment : public Texture
{
	friend class DiligentContext;
	friend class RenderTexture;
	
	TextureView m_rtv;

protected:
	RenderTextureAttachment() : Texture() {}
	RenderTextureAttachment(const RenderTextureDesc& desc, bool isDepth);
	
	const TextureView& GetRenderTargetView() const { return m_rtv; }
	void Create(const RenderTextureDesc& desc, bool isDepth);
};

class RenderTexture
{
	friend class DiligentContext;
	
	RenderTextureDesc m_description;
	
	std::vector<RenderTextureAttachment> m_attachments;
	std::vector<TextureView> m_cachedRtvs;

	explicit RenderTexture(const RenderTextureDesc& desc, size_t numColor, bool depth) noexcept;

protected:
	TextureView* GetColorRtvs() { return &m_cachedRtvs[1]; }
	TextureView GetDepthRtv() { return m_cachedRtvs[0]; }
	
public:
	static Shared<RenderTexture> Create(const RenderTextureDesc& desc, size_t numColor = 1, bool depth = true);

	RenderTexture(const RenderTexture& other) noexcept = delete;
	RenderTexture(RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture& other) noexcept = delete;
	~RenderTexture() = default;

	[[nodiscard]] uint32_t Width() const { return m_description.width; }
	[[nodiscard]] uint32_t Height() const { return m_description.height; }
};