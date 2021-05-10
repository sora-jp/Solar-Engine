#pragma once

#include <map>
#include <vector>
#include <string>

enum class CBufferBasicType
{
	Int, UInt, Float, Bool, Unknown
};

enum class CBufferViewType
{
	Scalar, Vector, Matrix, Array, Unknown
};

enum class ShaderType : uint32_t
{
	Unknown = 0x0000, ///< Unknown shader type
	Vertex = 0x0001, ///< Vertex shader
	Pixel = 0x0002, ///< Pixel (fragment) shader
	Geometry = 0x0004, ///< Geometry shader
	Hull = 0x0008, ///< Hull (tessellation control) shader
	DomainShader = 0x0010, ///< Domain (tessellation evaluation) shader
	Compute = 0x0020, ///< Compute shader
	Amplification = 0x0040, ///< Amplification (task) shader
	Mesh = 0x0080, ///< Mesh shader
	RayGen = 0x0100, ///< Ray generation shader
	RayMiss = 0x0200, ///< Ray miss shader
	RayClosestHit = 0x0400, ///< Ray closest hit shader
	RayAnyHit = 0x0800, ///< Ray any hit shader
	RayIntersection = 0x1000, ///< Ray intersection shader
	Callable = 0x2000, ///< Callable shader
};

inline ShaderType operator |(const ShaderType left, const ShaderType right)
{
	return static_cast<ShaderType>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
}

inline ShaderType& operator |=(ShaderType& left, const ShaderType right)
{
	return left = left | right;
}

struct CommonVariableData
{
	std::string name;
	ShaderType usages;
};

struct TextureVariable : public CommonVariableData
{

};

struct CBufferVariable : public CommonVariableData
{
	int byteOffset;
	int byteSize;

	CBufferBasicType baseType;
	CBufferViewType viewType;

	int baseTypeByteSize;
	int matrixCols, matrixRows, componentCount;
};

struct CBufferReflection : public CommonVariableData
{
	int index;
	int byteSize;
	std::map<std::string, CBufferVariable> variables;
	std::vector<CBufferVariable> rawVars;
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