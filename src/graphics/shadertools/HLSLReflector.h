#pragma once

#include <diligent/Graphics/GraphicsEngine/interface/Shader.h>

#include "shadertools/HLSLReflectionTypes.h"
#include "core/Log.h"

class HLSLReflector
{
public:
	static bool Reflect(std::string filename, std::string vs, std::string ps, std::string cs, Diligent::IShaderSourceInputStreamFactory* sourceResolver, ReflectionResult& outResult);
};