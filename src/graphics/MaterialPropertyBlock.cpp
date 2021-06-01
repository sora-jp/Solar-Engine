#include "pch.h"
#include "MaterialPropertyBlock.h"

#include "GraphicsSubsystem.h"
#include "diligent/DiligentInit.h"
#include "diligent/Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp"
#include "diligent/Graphics/GraphicsEngineD3D11/interface/ShaderResourceBindingD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3DBase/interface/ShaderResourceVariableD3D.h"
#include "Texture.h"

MaterialPropertyBlock::~MaterialPropertyBlock()
{
	delete[] m_backing;
}

inline int RoundUpBufSize(const int numToRound)
{
	return (numToRound + 15) & -16;
}

Unique<MaterialPropertyBlock> MaterialPropertyBlock::Create(Shared<Shader> shader)
{
	auto mpb = MakeUnique<MaterialPropertyBlock>();
	//mpb->m_shaderInfo = shader->m_reflectionInfo;

	shader->m_pipelineState->CreateShaderResourceBinding(&mpb->m_resourceBinding, false);

	auto* globals = shader->m_reflectionInfo.GetBuffer("_PerMaterial");

	if (globals) {
		const auto size = RoundUpBufSize(globals->byteSize);
		
		mpb->m_hasGlobals = true;
		mpb->m_globalsData = *globals;
		mpb->m_backing = new uint8_t[size];
		memset(mpb->m_backing, 0, size);
		
		BufferDesc buf;
		buf.Name = "MPB";
		buf.uiSizeInBytes = RoundUpBufSize(size);
		buf.Usage = USAGE_DEFAULT;
		buf.Mode = BUFFER_MODE_RAW;
		buf.BindFlags = BIND_UNIFORM_BUFFER;
		buf.CPUAccessFlags = CPU_ACCESS_NONE;

		GraphicsSubsystem::GetContext()->GetDevice()->CreateBuffer(buf, nullptr, &mpb->m_globalsBuffer);
		auto* var = mpb->m_resourceBinding->GetVariableByName(static_cast<SHADER_TYPE>(mpb->m_globalsData.usages), mpb->m_globalsData.name.c_str());
		if (var != nullptr) var->Set(mpb->m_globalsBuffer);
	}

	mpb->m_resourceBinding->InitializeStaticResources();
	return std::move(mpb);
}

bool MaterialPropertyBlock::SetTexture(const std::string& name, Texture* val, bool write)
{
	const auto& d = m_resourceBinding->GetPipelineState()->GetDesc();

	auto* srv = static_cast<Diligent::ITextureView*>(val->srv);
	if (d.IsAnyGraphicsPipeline()) 
	{
		auto* var = m_resourceBinding->GetVariableByName(SHADER_TYPE_PIXEL, name.c_str());
		if (var)
		{
			var->Set(srv);
			return true;
		}

		var = m_resourceBinding->GetVariableByName(SHADER_TYPE_VERTEX, name.c_str());
		if (var)
		{
			var->Set(srv);
			return true;
		}
	}

	if (d.IsComputePipeline()) 
	{	
		auto* var = m_resourceBinding->GetVariableByName(SHADER_TYPE_COMPUTE, name.c_str());
		if (var)
		{
			var->Set(srv);
			return true;
		}
	}
	
	return false;
}

void MaterialPropertyBlock::Flush()
{
	GraphicsSubsystem::GetContext()->GetContext()->UpdateBuffer(m_globalsBuffer, 0, m_globalsBuffer->GetDesc().uiSizeInBytes, m_backing, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void MaterialPropertyBlock::WriteGlobal(const CBufferVariable& var, void* val)
{
	//memcpy(m_backing + var.byteOffset, val, var.byteSize);
	Flush();
}
