#pragma once

#include <map>
#include <vector>
#include <string>
#include <diligent/Graphics/GraphicsEngine/interface/Shader.h>

struct CommonVariableData
{
	std::string name;
	Diligent::SHADER_TYPE usages;
};

struct TextureVariable : public CommonVariableData
{

};

struct CBufferVariable : public CommonVariableData
{
	int byteOffset;
	int byteSize;
};

struct CBufferReflection : public CommonVariableData
{
	int index;
	int byteSize;
	std::map<std::string, CBufferVariable> variables;
};

struct ReflectionResult
{
	std::vector<CBufferReflection> buffers;
	std::vector<TextureVariable> textures;

	CBufferReflection* GetBuffer(const std::string& name)
	{
		const auto it = std::find_if(buffers.begin(), buffers.end(), [&](CBufferReflection& b) {return b.name == name; });
		if (it == buffers.end()) return nullptr;
		return &(*it);
	}

	CommonVariableData* GetData(const size_t idx)
	{
		if (idx < buffers.size()) return &buffers[idx];
		if (idx - buffers.size() < textures.size()) return &textures[idx - buffers.size()];
		return nullptr;
	}

	[[nodiscard]] bool IsBuffer(const size_t idx) const
	{
		return idx < buffers.size();
	}
};