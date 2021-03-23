#include "pch.h"
#include "ScopedRendering.h"

ScopedRenderingContext::ScopedRenderingContext(const Shared<DiligentContext>& ctx, RenderTexture& rt) : m_target(rt)
{
	m_ctx = ctx;
}

void ScopedRenderingContext::BindRenderTarget() const
{
	m_ctx->SetRenderTarget(m_target);
}

void ScopedRenderingContext::Clear(float* rgba, float depth, int stencil) const
{
	m_ctx->ClearRenderTexture(m_target, rgba, depth, stencil);
}

void ScopedRenderingContext::End() const
{
	delete this;
}