#include "pch.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "core/Assert.h"
#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"
#include <filesystem>

using namespace Assimp;

Unique<SubMesh> SubMesh::LoadFrom(const aiMesh* m, const Shared<Mesh> parentMesh)
{
	auto mesh = MakeUnique<SubMesh>();

	mesh->materialIndex = m->mMaterialIndex;
	mesh->m_parentMesh = parentMesh;
	
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

		vs[i].tan.x = m->mTangents[i].x;
		vs[i].tan.y = m->mTangents[i].y;
		vs[i].tan.z = m->mTangents[i].z;

		if (m->HasTextureCoords(0))
		{
			vs[i].uv.x = m->mTextureCoords[0][i].x;
			vs[i].uv.y = m->mTextureCoords[0][i].y;
		}

		min = glm::min(min, vs[i].pos);
		max = glm::max(max, vs[i].pos);
	}

	for (auto i = 0; i < m->mNumFaces; i++)
	{
		is[i * 3 + 0] = m->mFaces[i].mIndices[0];
		is[i * 3 + 1] = m->mFaces[i].mIndices[1];
		is[i * 3 + 2] = m->mFaces[i].mIndices[2];
	}
	
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

	return std::move(mesh);
}

Shared<Texture2D> LoadTexture(const std::string& filename, aiMaterial* mat, const char* key, aiTextureType type, unsigned idx)
{
	aiString texPath;
	mat->Get(key, type, idx, texPath);

	if (texPath.length > 0)
	{
		//SOLAR_CORE_INFO("Loading texture {}", texPath.C_Str());
		return Texture2D::Load((std::filesystem::path(filename).parent_path() / texPath.C_Str()).string());
	}

	return nullptr;
}

Shared<Mesh> Mesh::Load(const std::string& filename)
{
	Importer imp;
	const auto* scene = imp.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_FindInvalidData | aiProcess_PreTransformVertices | aiProcess_OptimizeMeshes | aiProcess_FixInfacingNormals | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	
	SOLAR_CORE_ASSERT_ALWAYS(scene != nullptr);
	SOLAR_CORE_ASSERT(scene->HasMeshes());

	auto mesh = MakeShared<Mesh>();

	if (scene->HasMaterials()) 
	{
		for (auto i = 0u; i < scene->mNumMaterials; i++)
		{
			auto* mat = scene->mMaterials[i];

			SOLAR_CORE_INFO("{}", mat->GetName().C_Str());

			for (auto j = 0u; j < mat->mNumProperties; j++)
			{
				auto* prop = mat->mProperties[j];
				
				if (prop->mType == aiPTI_Float)
				{
					//SOLAR_CORE_INFO("\t{} = {}", prop->mKey.C_Str(), *reinterpret_cast<float*>(prop->mData));
				}
				else if (strcmp(prop->mKey.C_Str(), "$tex.file") == 0 && prop->mSemantic != aiTextureType_NONE) 
				{
					//if (prop->mSemantic == aiTextureType_DIFFUSE || prop->mSemantic == aiTextureType_NORMALS) continue;
					SOLAR_CORE_INFO("\tTEX: {} = {} ({})", prop->mKey.C_Str(), reinterpret_cast<aiString*>(prop->mData)->C_Str(), TextureTypeToString(static_cast<aiTextureType>(prop->mSemantic)));
				}
			}

			MeshMaterialData data;

			aiColor3D tmp;
			mat->Get(AI_MATKEY_COLOR_DIFFUSE, tmp);
			data.diffuse.r = tmp.r;
			data.diffuse.g = tmp.g;
			data.diffuse.b = tmp.b;

			mat->Get(AI_MATKEY_COLOR_EMISSIVE, tmp);
			data.emissive.r = tmp.r;
			data.emissive.g = tmp.g;
			data.emissive.b = tmp.b;

			data.diffuseTex = LoadTexture(filename, mat, AI_MATKEY_TEXTURE_DIFFUSE(0));
			data.normalTex = LoadTexture(filename, mat, AI_MATKEY_TEXTURE_NORMALS(0));
			data.metalRoughTex = LoadTexture(filename, mat, "$tex.file", aiTextureType_UNKNOWN, 0);
			
			
			//mat->Get(AI_MATKEY_SHININESS, data.roughness);
			//data.roughness = 1 - data.roughness;
			
			mat->Get("$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, data.roughness);
			mat->Get("$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, data.metallicity);

			mesh->m_materials.push_back(data);
		}
	}

	//if (scene->HasTextures())
	//{
	//	for (auto i = 0u; i < scene->mNumTextures; i++)
	//	{
	//		auto* tex = scene->mTextures[i];
	//		SOLAR_CORE_TRACE("Found texture \"{}\"", tex->mFilename.C_Str());
	//	}
	//}
	//
	for (auto i = 0u; i < scene->mNumMeshes; i++)
	{
		mesh->m_subMeshes.push_back(SubMesh::LoadFrom(scene->mMeshes[i], mesh));
	}
	
	return mesh;
}