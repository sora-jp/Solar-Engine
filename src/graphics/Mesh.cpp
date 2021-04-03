#include "pch.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "core/Assert.h"
#include "GraphicsSubsystem.h"

using namespace Assimp;

Shared<Mesh> Mesh::Load(std::string filename)
{
	Importer imp;
	auto* scene = imp.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData | aiProcess_PreTransformVertices | aiProcess_OptimizeMeshes);
	SOLAR_CORE_ASSERT_ALWAYS(scene != nullptr);
	SOLAR_CORE_ASSERT(scene->HasMeshes());

	const auto* m = scene->mMeshes[0];
	auto* const vs = new Vertex[m->mNumVertices];
	auto* const is = new uint32_t[m->mNumFaces * 3u];

	auto min = glm::vec3(INFINITY);
	auto max = glm::vec3(-INFINITY);
	
	for (auto i = 0; i < m->mNumVertices; i++)
	{
		vs[i].pos.x = m->mVertices[i].x;
		vs[i].pos.y = m->mVertices[i].y;
		vs[i].pos.z = m->mVertices[i].z;

		vs[i].nrm.x = m->mNormals[i].x;
		vs[i].nrm.y = m->mNormals[i].y;
		vs[i].nrm.z = m->mNormals[i].z;

		vs[i].uv.x = m->mTextureCoords[0][i].x;
		vs[i].uv.y = m->mTextureCoords[0][i].y;

		min = glm::min(min, vs[i].pos);
		max = glm::max(max, vs[i].pos);
	}

	for (auto i = 0; i < m->mNumFaces; i++)
	{
		is[i * 3 + 0] = m->mFaces[i].mIndices[0];
		is[i * 3 + 1] = m->mFaces[i].mIndices[1];
		is[i * 3 + 2] = m->mFaces[i].mIndices[2];
	}

	auto mesh = MakeShared<Mesh>();
	mesh->m_idxCount = m->mNumFaces * 3;
	mesh->bounds = { min, max };
	
	BufferDesc vsDesc;
	vsDesc.Name = "VS Buf";
	vsDesc.BindFlags = BIND_VERTEX_BUFFER;
	vsDesc.Usage = USAGE_IMMUTABLE;
	vsDesc.uiSizeInBytes = m->mNumVertices * sizeof(Vertex);

	BufferData vsData;
	vsData.DataSize = vsDesc.uiSizeInBytes;
	vsData.pData = vs;
	
	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateBuffer(vsDesc, &vsData, &mesh->m_vertBuf);

	BufferDesc isDesc;
	isDesc.Name = "IS Buf";
	isDesc.BindFlags = BIND_INDEX_BUFFER;
	isDesc.Usage = USAGE_IMMUTABLE;
	isDesc.uiSizeInBytes = m->mNumFaces * 3 * sizeof(int);

	BufferData isData;
	isData.DataSize = isDesc.uiSizeInBytes;
	isData.pData = is;

	GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateBuffer(isDesc, &isData, &mesh->m_idxBuf);
	
	delete[] vs;
	delete[] is;
	
	return mesh;
}
