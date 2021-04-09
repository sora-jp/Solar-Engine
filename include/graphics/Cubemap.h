#pragma once

#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Texture.h"

class Cubemap
{
	Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;

public:
	static Shared<Cubemap> Load(const std::string& file);

	[[nodiscard]] Diligent::ITextureView* GetTextureView();
};