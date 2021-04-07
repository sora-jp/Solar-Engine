#pragma once
#include "core/Common.h"
#include "core/Log.h"
#include "Material.h"

class MaterialPropertyBlock
{
	friend class DiligentContext;

	bool m_hasGlobals = false;
	CBufferReflection m_globalsData;
	RefCntAutoPtr<IShaderResourceBinding> m_resourceBinding;
	RefCntAutoPtr<IBuffer> m_globalsBuffer;
	uint8_t* m_backing = nullptr;

public:
	~MaterialPropertyBlock();
	
	static Unique<MaterialPropertyBlock> Create(Shared<Shader> shader);

	template<typename T, std::enable_if_t<std::is_base_of_v<IDeviceObject, T>, bool> = true>
	bool Set(const std::string& name, T* val)
	{
		auto* var = m_resourceBinding->GetVariableByName(SHADER_TYPE_PIXEL, name.c_str());
		if (!var)
			return false;

		var->Set(val);
		return true;
	}
	
	template <typename T, std::enable_if_t<!std::is_base_of_v<IDeviceObject, T>, bool> = true>
	bool Set(const std::string& name, T* val)
	{
		if (!m_hasGlobals || !m_globalsData.variables.count(name)) return false;
		auto& var = m_globalsData.variables[name];
		
		SOLAR_CORE_ASSERT(sizeof(T) == var.byteSize);

		WriteGlobal(var, val);
		
		return true;
	}

	template <typename T>
	bool Set(const std::string& name, T val)
	{
		return Set<T>(name, &val);
	}

private:
	void WriteGlobal(const CBufferVariable& var, void* val);
};