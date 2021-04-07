#pragma once
#include "core/Bounds.h"
#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Buffer.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "assimp/scene.h"

using namespace Diligent;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec2 uv;
};

class SubMesh
{
	friend class DiligentContext;
	friend class Mesh;
	
	uint32_t m_idxCount;
	RefCntAutoPtr<IBuffer> m_vertBuf;
	RefCntAutoPtr<IBuffer> m_idxBuf;

	static Unique<SubMesh> LoadFrom(const aiMesh* m);

public:
	Bounds bounds;
};

class Mesh
{
	friend class DiligentContext;

	std::vector<Unique<SubMesh>> m_subMeshes;
	
public:
	Bounds bounds;
	static Shared<Mesh> Load(std::string filename);

	int GetSubMeshCount() const { return m_subMeshes.size(); }
	const SubMesh& GetSubMesh(const int idx) const { return *m_subMeshes[idx]; }
};