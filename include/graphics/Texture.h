#pragma once
#include "core/Common.h"
#include <cstdint>

enum class TextureFormat
{
	Unknown, RGBA16, RGBA32, RGB32, RG32, RGBA8, RGBA8_SRGB, D16, D32, D32_S8, D24_S8
};

enum class TextureType
{
	Unknown, Tex2D, Tex2DArray, Tex3D, TexCube 
};

struct FullTextureDescription
{
	TextureFormat format = TextureFormat::Unknown;
	
	uint32_t width = 0, height = 0;
	union { uint32_t arraySize = 1; uint32_t depth; };
	
	uint32_t mipLevels = 1;
	bool generateMips = false;
	bool cpuReadWriteEnabled = false;
	bool isRenderTarget = false;
};

class Texture
{
	friend class DiligentContext;

protected:
	FullTextureDescription description;
	void* srv;
	void* texHandle;

	Texture() noexcept : srv(nullptr), texHandle(nullptr) {}
	explicit Texture(TextureType type, const FullTextureDescription& desc) noexcept;
	explicit Texture(TextureType type, const std::string& path) noexcept;
	
	void* GetResourceView() const { return srv; }

public:
	Texture(const Texture& other) noexcept = delete;
	Texture(Texture&& other) noexcept = delete;
	Texture& operator=(const Texture&& other) noexcept = delete;
	Texture& operator=(const Texture& other) noexcept = delete;
	
	~Texture();
	
	[[nodiscard]] uint32_t Width() const { return description.width; }
	[[nodiscard]] uint32_t Height() const { return description.height; }
};

class Texture2D_ : public Texture
{
	explicit Texture2D_(const FullTextureDescription& desc) : Texture(TextureType::Tex2D, desc) {}
	explicit Texture2D_(const std::string& data) : Texture(TextureType::Tex2D, data) {}
	
public:
	static Shared<Texture2D_> Create(const FullTextureDescription& desc) { return Shared<Texture2D_>(new Texture2D_(desc)); }
	static Shared<Texture2D_> Load(const std::string& file) { return Shared<Texture2D_>(new Texture2D_(file)); }
};

class TextureCube : public Texture
{
	explicit TextureCube(const FullTextureDescription& desc) : Texture(TextureType::TexCube, desc) {}
	explicit TextureCube(const std::string& data) : Texture(TextureType::TexCube, data) {}

public:
	static Shared<TextureCube> Create(const FullTextureDescription& desc) { return Shared<TextureCube>(new TextureCube(desc)); }
	static Shared<TextureCube> Load(const std::string& file) { return Shared<TextureCube>(new TextureCube(file)); }
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
	
	void* m_rtv;

protected:
	RenderTextureAttachment() : m_rtv(nullptr) {}
	RenderTextureAttachment(const RenderTextureDesc& desc, bool isDepth);
	
	void* GetRenderTargetView() const { return m_rtv; }
};

class RenderTexture
{
	friend class DiligentContext;
	
	RenderTextureDesc m_description;
	
	std::vector<RenderTextureAttachment*> m_attachments;
	std::vector<void*> m_cachedRtvs;

	explicit RenderTexture(const RenderTextureDesc& desc, size_t numColor, bool depth) noexcept;

protected:
	void* const* GetColorRtvs() const { return &m_cachedRtvs[1]; }
	void* GetDepthRtv() const { return m_cachedRtvs[0]; }
	
public:
	static Shared<RenderTexture> Create(const RenderTextureDesc& desc, size_t numColor = 1, bool depth = true);

	RenderTexture(const RenderTexture& other) noexcept = delete;
	RenderTexture(RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture& other) noexcept = delete;
	~RenderTexture();

	[[nodiscard]] uint32_t Width() const { return m_description.width; }
	[[nodiscard]] uint32_t Height() const { return m_description.height; }

	const RenderTextureAttachment* Color(const size_t idx) const { return m_attachments[idx + 1]; }
	const RenderTextureAttachment* Depth() const { return m_attachments[0]; }
};