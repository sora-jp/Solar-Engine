#pragma once
#include "Common.h"
#include <vector>

template <typename Base> class TypeRegistry
{
public:
	typedef Base base;
	
	static std::vector<Shared<Base>> Instances;
	static bool Register(Shared<Base> instance)
	{
		Instances.push_back(instance);
		SOLAR_TRACE("Register() {}", (void*)Instances.back().get());
		return true;
	};

	static const std::vector<Shared<Base>>& Get()
	{
		return Instances;
	}
};

template<typename Factory, typename Type, std::enable_if_t<std::is_base_of_v<typename Factory::base, Type>, bool> = true> class TypeRegistrar
{
public:
	TypeRegistrar()
	{
		Factory::Register(MakeShared<Type>());
	};
};

template<typename Base> std::vector<Shared<Base>> TypeRegistry<Base>::Instances;

#define GET(factory) factory::Get()
#define REGISTER(factory, t) static TypeRegistrar<factory, t> s_ ##t ##Registrar;

#define INSTANTIATE_FACTORY(factorytype) template<> class TypeRegistry<factorytype>  \
{																			         \
public:																		         \
	typedef BaseSystem base;												         \
	static std::vector<Shared<factorytype>> Instances;								         \
	static const std::vector<Shared<factorytype>>& Get() { return Instances; }				     \
	static bool Register(Shared<factorytype> s)									         \
	{																		         \
		Instances.push_back(s);												         \
		return true;														         \
	}																		         \
};																			         \
																			         \
std::vector<Shared<factorytype>> TypeRegistry<factorytype>::Instances