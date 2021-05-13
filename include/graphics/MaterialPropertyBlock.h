#pragma once
#include "Material.h"
#include "TextureBase.h"
#include "core/Common.h"
#include "core/Assert.h"
#include "core/Log.h"

class MaterialPropertyBlock
{
	friend class DiligentContext;

	bool m_hasGlobals = false;
	CBufferReflection m_globalsData;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_resourceBinding;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> m_globalsBuffer;
	uint8_t* m_backing = nullptr;

public:
	~MaterialPropertyBlock();
	
	static Unique<MaterialPropertyBlock> Create(Shared<Shader> shader);

	bool SetTexture(const std::string& name, TextureBase val, bool write = false);

	template <typename T, std::enable_if_t<!std::is_base_of_v<Diligent::IDeviceObject, T> && std::is_pod_v<T> && !std::is_pointer_v<T>, bool> = true>
	bool Set(const std::string& name, T* val)
	{
		if (!m_hasGlobals || !m_globalsData.variables.count(name)) return false;
		auto& var = m_globalsData.variables[name];
		
		SOLAR_CORE_ASSERT(sizeof(T) == var.byteSize);

		WriteGlobal(var, val);
		
		return true;
	}

	template <typename T, std::enable_if_t<!std::is_base_of_v<Diligent::IDeviceObject, std::remove_all_extents_t<T>> && std::is_pod_v<std::remove_all_extents_t<T>> && !std::is_pointer_v<T>, bool> = true>
	bool Set(const std::string& name, T val)
	{
		if (!m_hasGlobals || !m_globalsData.variables.count(name)) return false;
		auto& var = m_globalsData.variables[name];

		SOLAR_CORE_ASSERT(sizeof(T) == var.byteSize);

		WriteGlobal(var, &val);

		return true;
	}

	template<typename T>
	T* Property(const std::string& name)
	{
		//if (!m_hasGlobals || !m_globalsData.variables.count(name)) return T();
		auto& var = m_globalsData.variables[name];

		//SOLAR_CORE_ASSERT(sizeof(T) == var.byteSize);
		return reinterpret_cast<T*>(m_backing + var.byteOffset);
	}

	void Iterate(std::function<void(CBufferVariable&)> func)
	{
		if (!m_hasGlobals) return;
		for (auto& desc : m_globalsData.rawVars)
		{
			func(desc);
		}
	}

	void Flush();

private:
	void WriteGlobal(const CBufferVariable& var, void* val);
};