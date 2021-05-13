#include "pch.h"
#include "shadertools/HLSLReflector.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"

using namespace glslang;
using namespace Diligent;

class DiligentIncluder : public TShader::Includer
{
	IShaderSourceInputStreamFactory* m_resolver;

public:
	explicit DiligentIncluder(IShaderSourceInputStreamFactory* res) : m_resolver(res) {}
	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t includeDepth) override;
	void releaseInclude(IncludeResult* include) override;
};

inline bool AddShader(std::vector<TShader*>& shaders, const char* const* src, const std::string& entry, EShLanguage type, DiligentIncluder& includer)
{
	auto* shd = new TShader(type);
	shd->setStrings(src, 1);
	shd->setEnvTargetHlslFunctionality1();
	shd->setEnvInput(EShSourceHlsl, type, EShClientNone, 0);
	shd->setEnvClient(EShClientNone, EShTargetClientVersionCount);
	shd->setEnvTarget(EShTargetNone, EShTargetLanguageVersionCount);
	shd->setEntryPoint(entry.c_str());

	TBuiltInResource r{};

	if (shd->parse(&r, 0, false, static_cast<EShMessages>(EShMsgReadHlsl | EShMsgDefault), includer))
		shaders.push_back(shd);
	else
	{
		SOLAR_CORE_WARN("Shader compilation failed.");
		SOLAR_CORE_WARN(shd->getInfoLog());
		return false;
	}

	return true;
}

inline ShaderType MapToShaderType(EShLanguageMask mask)
{
	auto out = static_cast<ShaderType>(0);
	if (mask & EShLangVertexMask) out |= ShaderType::Vertex;
	if (mask & EShLangFragmentMask) out |= ShaderType::Pixel;
	if (mask & EShLangComputeMask) out |= ShaderType::Compute;

	return out;
}

inline int GetByteSize(const TBasicType type)
{
	switch (type)
	{
		case EbtInt8:
		case EbtUint8:
		case EbtBool:
			return 1;
			
		case EbtFloat16:
		case EbtInt16:
		case EbtUint16:
			return 2;
			
		case EbtInt:
		case EbtUint:
		case EbtFloat:
			return 4;
			
		case EbtInt64:
		case EbtUint64:
		case EbtDouble:
			return 8;
			
		default: 
			return 0;
	}
}

inline int GetByteSize(const TType* type)
{
	const auto basicSize = GetByteSize(type->getBasicType());
	
	if (type->isScalarOrVec1()) return basicSize;
	if (type->isVector()) return basicSize * type->getVectorSize();
	if (type->isMatrix()) return basicSize * type->getMatrixCols() * type->getMatrixRows();

	return 0;
}

inline CBufferBasicType GetBasicType(const TType* type)
{
	switch (type->getBasicType())
	{
		case EbtInt8:
		case EbtInt16:
		case EbtInt:
		case EbtInt64:
			return CBufferBasicType::Int;
		
		case EbtUint8:
		case EbtUint16:
		case EbtUint:
		case EbtUint64:
			return CBufferBasicType::UInt;

		case EbtBool:
			return CBufferBasicType::Bool;

		case EbtFloat16:
		case EbtFloat:
		case EbtDouble:
			return CBufferBasicType::Float;

		default:
			return CBufferBasicType::Unknown;
	}
}

inline CBufferViewType GetViewType(const TType* type)
{
	if (type->isScalarOrVec1()) return CBufferViewType::Scalar;
	if (type->isVector()) return CBufferViewType::Vector;
	if (type->isMatrix()) return CBufferViewType::Matrix;
	if (type->isArray()) return CBufferViewType::Array;

	return CBufferViewType::Unknown;
}

bool HLSLReflector::Reflect(std::string filename, std::string vs, std::string ps, std::string cs, IShaderSourceInputStreamFactory* sourceResolver, ReflectionResult& outResult)
{
	auto includer = DiligentIncluder(sourceResolver);
	auto* src = includer.includeLocal(filename.c_str(), nullptr, 0);

	//SOLAR_CORE_INFO("Reflecting on shader {}", filename);

	std::vector<TShader*> shaders;

	auto success = true;
	if (!vs.empty()) success &= AddShader(shaders, &src->headerData, vs, EShLangVertex, includer);
	if (!ps.empty()) success &= AddShader(shaders, &src->headerData, ps, EShLangFragment, includer);
	if (!cs.empty()) success &= AddShader(shaders, &src->headerData, cs, EShLangCompute, includer);

	if (success)
	{
		TProgram prog;
		for (auto* s : shaders) prog.addShader(s);

		if (prog.link(static_cast<EShMessages>(EShMsgReadHlsl | EShMsgDebugInfo | EShMsgDefault)))
		{
			if (prog.buildReflection(EShReflectionAllBlockVariables | EShReflectionDefault | EShReflectionSeparateBuffers | EShReflectionAllIOVariables))
			{
				prog.dumpReflection();
				
				const auto numUBOs = prog.getNumUniformBlocks();
				outResult.buffers.resize(numUBOs);

				for (auto i = 0; i < numUBOs; i++)
				{
					const auto& block = prog.getUniformBlock(i);
					outResult.buffers[i] = CBufferReflection
					{
						block.name,
						MapToShaderType(block.stages),
						block.index,
						block.size,
						std::map<std::string, CBufferVariable>()
					};
				}

				const auto numUniforms = prog.getNumUniformVariables();

				for (auto i = 0; i < numUniforms; i++)
				{
					const auto& uniform = prog.getUniform(i);
					const auto* type = uniform.getType();
					const auto size = GetByteSize(type);
					
					SOLAR_CORE_INFO("Uniform {}: {} (size: {}, offset: {})", uniform.name.c_str(), uniform.getType()->getCompleteString().c_str(), size, uniform.offset);
					
					if (uniform.index != -1)
					{
						CBufferVariable v = {
							{uniform.name, MapToShaderType(uniform.stages)},
							uniform.offset, size,
							GetBasicType(type), GetViewType(type),
							GetByteSize(type->getBasicType()),
							type->getMatrixCols(), type->getMatrixRows(), type->getVectorSize()
						};
						
						outResult.buffers[uniform.index].variables[uniform.name] = v;
						outResult.buffers[uniform.index].rawVars.push_back(v);
					}
					else if (uniform.getType()->isTexture() || uniform.getType()->isImage())
					{
						outResult.textures.push_back({
							{ uniform.name, MapToShaderType(uniform.stages) }
						});
					}
				}

				for (auto& buf : outResult.buffers)
				{	
					std::sort(
						buf.rawVars.begin(),
						buf.rawVars.end(),
						[](const CBufferVariable& a, const CBufferVariable& b){ return a.byteOffset < b.byteOffset; });
				}
			}
			else
			{
				SOLAR_CORE_WARN("Reflection failed for {}", filename);
				SOLAR_CORE_WARN(prog.getInfoLog());
				success = false;
			}
		}
		else
		{
			SOLAR_CORE_WARN("Program linking for {} failed", filename);
			SOLAR_CORE_WARN(prog.getInfoLog());
			success = false;
		}
	}

	for (auto* s : shaders) delete s;

	includer.releaseInclude(src);
	return success;
}

inline TShader::Includer::IncludeResult* DiligentIncluder::includeLocal(const char* headerName, const char* includerName, size_t includeDepth)
{
	IFileStream* stream;
	m_resolver->CreateInputStream(headerName, &stream);

	const auto size = stream->GetSize();
	auto* buf = new char[size + 1];

	if (!stream->Read(buf, size)) {
		SOLAR_CORE_WARN("INCLUDE FAILED: {} (depth {})", headerName, includeDepth);
		return nullptr;
	}

	buf[size] = 0;
	stream->Release();
	return new IncludeResult(headerName, buf, size, nullptr);
}

inline void DiligentIncluder::releaseInclude(IncludeResult* include)
{
	delete[] include->headerData;
	delete include;
}
