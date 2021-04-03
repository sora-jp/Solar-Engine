#pragma once
#include "core/Bounds.h"
#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Buffer.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

using namespace Diligent;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec2 uv;
};

class Mesh
{
	friend class DiligentContext;

	uint32_t m_idxCount;
	RefCntAutoPtr<IBuffer> m_vertBuf;
	RefCntAutoPtr<IBuffer> m_idxBuf;
	
public:
	Bounds bounds;
	static Shared<Mesh> Load(std::string filename);
};