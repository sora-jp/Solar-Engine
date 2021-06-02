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
	friend class MaterialPropertyBlock;

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

	[[nodiscard]] const FullTextureDescription& Description() const { return description; }
	[[nodiscard]] uint32_t Width() const { return description.width; }
	[[nodiscard]] uint32_t Height() const { return description.height; }
	[[nodiscard]] void* RawTextureHandle() const { return texHandle; }
	[[nodiscard]] void* RawResourceHandle() const { return srv; }
};

class Texture2D : public Texture
{
	explicit Texture2D(const FullTextureDescription& desc) : Texture(TextureType::Tex2D, desc) {}
	explicit Texture2D(const std::string& data) : Texture(TextureType::Tex2D, data) {}
	
public:
	static Shared<Texture2D> Create(const FullTextureDescription& desc) { return Shared<Texture2D>(new Texture2D(desc)); }
	static Shared<Texture2D> Load(const std::string& file) { return Shared<Texture2D>(new Texture2D(file)); }
};

class TextureCube : public Texture
{
	explicit TextureCube(const FullTextureDescription& desc) : Texture(TextureType::TexCube, desc) {}
	explicit TextureCube(const std::string& data);

public:
	static Shared<TextureCube> Create(const FullTextureDescription& desc) { return Shared<TextureCube>(new TextureCube(desc)); }
	static Shared<TextureCube> Load(const std::string& file) { return Shared<TextureCube>(new TextureCube(file)); }
	static Shared<TextureCube> ConvolveDiffuse(const Shared<TextureCube>& other);
};

struct RenderTextureDesc
{
	uint32_t width = 0, height = 0;
	TextureFormat colorBufferFormat = TextureFormat::RGBA32;
	TextureFormat depthBufferFormat = TextureFormat::D24_S8;
};

class RenderTarget
{
	friend class DiligentContext;

	void** GetColorRtvs() { return colorRtvs.data(); };
	void* GetDepthRtv() { return depthRtv; };
	size_t GetColorRtvCount() const { return colorRtvs.size(); }

protected:
	std::vector<void*> colorRtvs;
	void* depthRtv;

public:
	[[nodiscard]] uint32_t Width() const;
	[[nodiscard]] uint32_t Height() const;
};

class RenderTextureAttachment : public Texture, public RenderTarget
{
	friend class DiligentContext;
	friend class RenderTexture;

protected:
	RenderTextureAttachment(const RenderTextureDesc& desc, bool isDepth);
};

class RenderTexture : public RenderTarget
{
	RenderTextureDesc m_description;
	
	std::vector<RenderTextureAttachment*> m_attachments;

	explicit RenderTexture(const RenderTextureDesc& desc, size_t numColor, bool depth) noexcept;

public:
	static Shared<RenderTexture> Create(const RenderTextureDesc& desc, size_t numColor = 1, bool depth = true);

	RenderTexture(const RenderTexture& other) noexcept = delete;
	RenderTexture(RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture&& other) noexcept = delete;
	RenderTexture& operator=(const RenderTexture& other) noexcept = delete;
	~RenderTexture();

	[[nodiscard]] uint32_t Width() const { return m_description.width; }
	[[nodiscard]] uint32_t Height() const { return m_description.height; }

	RenderTextureAttachment* Color(const size_t idx) const { return m_attachments[idx + 1]; }
	RenderTextureAttachment* Depth() const { return m_attachments[0]; }
};