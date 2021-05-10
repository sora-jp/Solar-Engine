#pragma once
#include "TextureBase.h"
#include "core/Bounds.h"
#include "core/Common.h"
#include "diligent/Common/interface/RefCntAutoPtr.hpp"
#include "diligent/Graphics/GraphicsEngine/interface/Buffer.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "assimp/scene.h"

//TODO: Remove references to Diligent from here
using namespace Diligent;

struct aiMesh;
struct Vertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec3 tan;
	glm::vec2 uv;
};

struct MeshMaterialData
{
	glm::vec3 diffuse;
	glm::vec3 emissive;
	float roughness;
	float metallicity;

	Shared<Texture2D> diffuseTex, normalTex, metalRoughTex;
};

class Mesh
{
	friend class DiligentContext;
	friend class SubMesh;

	std::vector<Unique<SubMesh>> m_subMeshes;
	std::vector<MeshMaterialData> m_materials;
	
public:
	Bounds bounds;
	static Shared<Mesh> Load(const std::string& filename);

	int GetSubMeshCount() const { return m_subMeshes.size(); }
	const SubMesh& GetSubMesh(const int idx) const { return *m_subMeshes[idx]; }

	int GetMaterialCount() const { return m_materials.size(); }
	const MeshMaterialData& GetMaterialData(const int idx) const { return m_materials[idx]; }
	const std::vector<MeshMaterialData>& GetMaterials() const { return m_materials; }
};

class SubMesh
{
	friend class DiligentContext;
	friend class Mesh;

	uint32_t m_idxCount;
	std::weak_ptr<Mesh> m_parentMesh;
	RefCntAutoPtr<IBuffer> m_vertBuf;
	RefCntAutoPtr<IBuffer> m_idxBuf;

	static Unique<SubMesh> LoadFrom(const aiMesh* m, Shared<Mesh> parentMesh);

public:
	Bounds bounds;
	int materialIndex;

	const MeshMaterialData& GetMaterial() const { return m_parentMesh.lock()->GetMaterialData(materialIndex); }
};