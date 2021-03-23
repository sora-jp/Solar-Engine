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
	ITextureView** m_rawColorTargets;
	RefCntAutoPtr<ITextureView>* m_colorTargets;
	RefCntAutoPtr<ITextureView> m_depthTarget;

	void Rebind(int numColorTargets, ITextureView** colorTargets, ITextureView* depthTarget);
	
public:
	RenderTexture(int numColorTargets, ITextureView* colorTargets[], ITextureView* depthTarget);
	~RenderTexture();
};

inline void RenderTexture::Rebind(int numColorTargets, ITextureView** colorTargets, ITextureView* depthTarget)
{
	m_numColorTargets = numColorTargets;
	for (auto i = 0; i < numColorTargets; i++) m_rawColorTargets[i] = m_colorTargets[i] = colorTargets[i];
	m_depthTarget = depthTarget;
}

inline RenderTexture::RenderTexture(const int numColorTargets, ITextureView* colorTargets[], ITextureView* depthTarget)
{
	m_numColorTargets = numColorTargets;
	m_colorTargets = new RefCntAutoPtr<ITextureView>[numColorTargets];
	m_rawColorTargets = new ITextureView*[numColorTargets];
	for (auto i = 0; i < numColorTargets; i++) m_rawColorTargets[i] = m_colorTargets[i] = colorTargets[i];
	m_depthTarget = depthTarget;
}

inline RenderTexture::~RenderTexture()
{
	delete[] m_colorTargets;
	delete[] m_rawColorTargets;
}