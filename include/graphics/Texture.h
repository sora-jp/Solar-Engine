#pragma once
#include "core/Common.h"
#include <cstdint>

enum class TextureFormat
{
	RGBA32, RGB32, RG32, RGBA8, RGBA8_SRGB, D16, D32, D32_S8, D24_S8
};

enum class TextureType
{
	Tex2D, Tex2DArray, Tex3D, TexCube 
};

struct _TextureDescription
{
	TextureType type;
	TextureFormat format;
	
	uint32_t width, height;
	union { uint32_t arraySize; uint32_t depth; };
	
	uint32_t mipLevels;
	bool isRenderTarget;

	bool cpuReadWriteEnabled;
};

class TextureView;
class TextureResource
{
	friend class DiligentContext;
	
protected:
	virtual const TextureView& GetView(bool renderTarget) = 0;
	virtual ~TextureResource() = default;
};

class TextureView : public TextureResource
{
	friend class Texture;

	bool m_isRenderTarget;
	bool m_shouldRelease;
	void* m_texViewHandle;

	TextureView(void* handle, const bool shouldRelease);

protected:
	const TextureView& GetView(bool renderTarget) override { return *this; }

public:
	~TextureView();
};

class Texture : public TextureResource
{
	_TextureDescription m_description;
	void* m_texHandle;

	Texture(const _TextureDescription& desc, void* handle) noexcept : m_description(desc), m_texHandle(handle) {}
	Texture(const Texture& other) noexcept = default;

protected:
	const TextureView& GetView(bool renderTarget) override;

public:
	static Shared<Texture> Create(const _TextureDescription& desc);
	virtual ~Texture();
	
	[[nodiscard]] uint32_t Width() const { return m_description.width; }
	[[nodiscard]] uint32_t Height() const { return m_description.height; }
};