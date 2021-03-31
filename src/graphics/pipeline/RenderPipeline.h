#pragma once
#include "diligent/DiligentInit.h"

class RenderPipeline
{
public:
	virtual void Render(DiligentContext const& ctx) = 0;
};
