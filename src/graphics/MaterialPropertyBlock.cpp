#include "pch.h"
#include "MaterialPropertyBlock.h"

#include "GraphicsSubsystem.h"
#include "diligent/Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp"
#include "diligent/Graphics/GraphicsEngineD3D11/interface/ShaderResourceBindingD3D11.h"
#include "diligent/Graphics/GraphicsEngineD3DBase/interface/ShaderResourceVariableD3D.h"

MaterialPropertyBlock::~MaterialPropertyBlock()
{
	delete[] m_backing;
}

Unique<MaterialPropertyBlock> MaterialPropertyBlock::Create(Shared<Shader> shader)
{
	auto mpb = MakeUnique<MaterialPropertyBlock>();
	//mpb->m_shaderInfo = shader->m_reflectionInfo;

	shader->m_pipelineState->CreateShaderResourceBinding(&mpb->m_resourceBinding, false);

	auto* globals = shader->m_reflectionInfo.GetBuffer("_PerMaterial");

	if (globals) {
		mpb->m_hasGlobals = true;
		mpb->m_globalsData = *globals;
		mpb->m_backing = new uint8_t[globals->byteSize];
		
		BufferDesc buf;
		buf.Name = "MPB";
		buf.uiSizeInBytes = globals->byteSize;
		buf.Usage = USAGE_DEFAULT;
		buf.Mode = BUFFER_MODE_RAW;
		buf.BindFlags = BIND_UNIFORM_BUFFER;
		buf.CPUAccessFlags = CPU_ACCESS_NONE;

		GraphicsSubsystem::GetCurrentContext()->GetDevice()->CreateBuffer(buf, nullptr, &mpb->m_globalsBuffer);
		mpb->m_resourceBinding->GetVariableByName(mpb->m_globalsData.usages, mpb->m_globalsData.name.c_str())->Set(mpb->m_globalsBuffer);
	}

	mpb->m_resourceBinding->InitializeStaticResources();
	return std::move(mpb);
}

void MaterialPropertyBlock::WriteGlobal(const CBufferVariable& var, void* val)
{
	memcpy(m_backing, val, var.byteSize);
	GraphicsSubsystem::GetCurrentContext()->GetContext()->UpdateBuffer(m_globalsBuffer, 0, m_globalsData.byteSize, m_backing, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}
