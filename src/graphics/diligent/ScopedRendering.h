#pragma once
#include "RenderTexture.h"
#include "DiligentInit.h"
#include "DiligentWindow.h"

using ScopedRenderingCtxPtr = Unique<class ScopedRenderingContext>;

class ScopedRenderingContext
{
	RenderTexture& m_target;
	Shared<DiligentContext> m_ctx;

public:
	ScopedRenderingContext(const Shared<DiligentContext>& ctx, RenderTexture& rt);
	~ScopedRenderingContext() = default;

	void BindRenderTarget() const;
	void Clear(float* rgba, float depth, int stencil) const;

	static ScopedRenderingCtxPtr Begin(const Shared<DiligentWindow>& window);
	void End() const;
};

inline ScopedRenderingCtxPtr ScopedRenderingContext::Begin(const Shared<DiligentWindow>& window)
{
	return MakeUnique<ScopedRenderingContext>(window->GetContext(), window->GetRenderTarget());
}