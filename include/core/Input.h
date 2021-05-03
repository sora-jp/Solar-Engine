#pragma once
#include "Common.h"
#include <map>
#include "entt/entt.hpp"

class InputDevice
{
	friend class Input;
	
public:
	InputDevice() = default;
	virtual ~InputDevice() = default;
	
protected:
	virtual void Update() = 0;
};

class Input
{
	Input() = delete;
	~Input() = delete;

	static std::map<entt::id_type, std::vector<Unique<InputDevice>>> _providers;
	
public:
	template<class T, std::enable_if_t<std::is_base_of_v<InputDevice, T>, bool> = true> static const T& First()
	{
		const auto& devs = _providers[entt::type_id<T>()];
		SOLAR_ASSERT(devs.size() > 0);
		return devs[0];
	}

	template<class T, std::enable_if_t<std::is_base_of_v<InputDevice, T>, bool> = true> static size_t All(std::vector<const T&> out)
	{
		out.clear();
		const auto& devs = _providers[entt::type_id<T>()];
		const auto last = std::transform(devs.cbegin(), devs.cend(), out.begin(), [](const Unique<InputDevice>& dev) { return *dev; });
		return std::distance(out.begin(), last);
	}

	template<class T, std::enable_if_t<std::is_base_of_v<InputDevice, T>, bool> = true, typename... Args> static void AddDevice(Args&&... args)
	{	
		_providers[entt::type_id<T>()].push_back(MakeUnique<T>(std::forward<Args>(args)...));
	}

	template<class T, std::enable_if_t<std::is_base_of_v<InputDevice, T>, bool> = true> static void RemoveDevice(const T& device)
	{
		auto& devs = _providers[entt::type_id<T>()];
		const auto* ptr = &device;
		devs.erase(std::find_if(devs.cbegin(), devs.cend(), [=](const Unique<InputDevice>& dev) { return dev.get() == ptr; }));
	}

	static void UpdateAll()
	{
		std::for_each(_providers.begin(), _providers.end(), [](auto& devs) { std::for_each(devs.begin(), devs.end(), &InputDevice::Update); });
	}
};