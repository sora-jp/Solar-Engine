#pragma once
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/TextureView.h"

using namespace Diligent;

struct RenderTexture
{
	friend class DiligentContext;
	friend class DiligentWindow;
	
private:
	int m_numColorTargets;
	ITextureView** m_rct;
	RefCntAutoPtr<ITextureView>* m_colorTargets;
	RefCntAutoPtr<ITextureView> m_depthTarget;

	void Rebind(int numColorTargets, ITextureView** colorTargets, ITextureView* depthTarget);
	
public:
	RenderTexture() : m_numColorTargets(-1), m_rct(nullptr), m_colorTargets(nullptr) {}
	RenderTexture(int numColorTargets, ITextureView* colorTargets[], ITextureView* depthTarget);
	~RenderTexture();

	[[nodiscard]] bool IsValid() const { return m_numColorTargets != 0; }
};

inline void RenderTexture::Rebind(int numColorTargets, ITextureView** colorTargets, ITextureView* depthTarget)
{
	m_numColorTargets = numColorTargets;
	for (auto i = 0; i < numColorTargets; i++) m_rct[i] = m_colorTargets[i] = colorTargets[i];
	m_depthTarget = depthTarget;
}

inline RenderTexture::RenderTexture(const int numColorTargets, ITextureView* colorTargets[], ITextureView* depthTarget)
{
	m_numColorTargets = numColorTargets;
	if (numColorTargets > 0) {
		m_colorTargets = new RefCntAutoPtr<ITextureView>[numColorTargets];
		m_rct = new ITextureView* [numColorTargets];
		for (auto i = 0; i < numColorTargets; i++) m_rct[i] = m_colorTargets[i] = colorTargets[i];
	}
	m_depthTarget = depthTarget;
}

inline RenderTexture::~RenderTexture()
{
	delete[] m_rct;
	delete[] m_colorTargets;
}