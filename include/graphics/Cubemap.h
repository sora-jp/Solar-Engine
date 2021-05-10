#pragma once

#include "TextureBase.h"
#include "core/Common.h"

class Cubemap : public TextureBase
{
public:
	static Shared<Cubemap> Load(const std::string& file);
};