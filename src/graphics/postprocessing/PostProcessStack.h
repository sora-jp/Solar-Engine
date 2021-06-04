#pragma once
#include "pipeline/PipelineContext.h"

struct PostProcessContext
{
	RenderTexture* gBuffers;
	const CameraComponent* camera;
	const PipelineContext* ctx;
	CullingResults* culled;
};

enum class EffectLocation
{
	Unknown, BeforeComposite, AfterComposite
};

class EffectBase
{
	friend class PostProcessStack;
	typedef void SettingsType;

protected:
	virtual EffectLocation Location() = 0;

public:
	virtual ~EffectBase() = default;
	virtual void Render(RenderTarget* target, PostProcessContext& ctx) = 0;
};


class PostProcessStack
{
	std::map<entt::id_type, EffectBase*> m_effects;
	std::vector<entt::id_type> m_execOrder;

public:
	template<typename T, std::enable_if_t<std::is_base_of_v<EffectBase, T>, bool> = true>
	void Use()
	{
		constexpr auto hash = entt::type_hash<T>::value();
		m_effects[hash] = new T();
		m_execOrder.push_back(hash);
	}

	template<typename T, std::enable_if_t<std::is_base_of_v<EffectBase, T> && !std::is_void_v<typename T::SettingsType>, bool> = true>
	typename T::SettingsType& Settings()
	{
		return m_effects[entt::type_id<T>().hash()]->m_settings;
	}

	template<typename T, std::enable_if_t<std::is_base_of_v<EffectBase, T>, bool> = true>
	const T& Get()
	{
		return *static_cast<T*>(m_effects[entt::type_id<T>().hash()]);
	}

	template<EffectLocation L>
	void Render(RenderTarget* target, PostProcessContext& ctx)
	{
		for (auto h : m_execOrder) 
		{
			if (m_effects[h]->Location() == L)
				m_effects[h]->Render(target, ctx);
		}
	}

	~PostProcessStack()
	{
		for (auto [id, eff] : m_effects)
		{
			delete eff;
		}
	}
};

template<typename T, EffectLocation loc>
class PostProcessEffect : public EffectBase
{
	friend PostProcessStack;
	
	typedef T SettingsType;
	constexpr EffectLocation Location() override { return loc; }
	
	T m_settings = T();

public:
	~PostProcessEffect() override = default;
	void Render(RenderTarget* target, PostProcessContext& ctx) override { Render(target, ctx, m_settings); }
	virtual void Render(RenderTarget* target, PostProcessContext& context, T settings) = 0;
};