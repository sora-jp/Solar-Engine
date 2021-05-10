#pragma once
#include <functional>
#include <map>

#define DECL_VFN(func)															 \
    template<typename T, typename Sign>											 \
    struct has_ ##func {                                                         \
        typedef char yes[1];													 \
        typedef char no [2];													 \
        template <typename U, U> struct type_check;								 \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func > *);  \
        template <typename   > static no  &chk(...);							 \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);				 \
    }

#define REGISTER_VFN(collection, fn) if constexpr (ComponentWrapper<T>::has_ ## fn<T, void(T::*)()>::value) (collection)[#fn] = &T::fn

class ComponentWrapperBase
{
public:
	virtual ~ComponentWrapperBase() = default;
	virtual void Call(void* instance, std::string fnName) = 0;
};

template<typename T>
class ComponentWrapper : ComponentWrapperBase
{	
	DECL_VFN(OnInspectorGUI);
	DECL_VFN(Update);
	
	static const std::map<std::string, std::function<void(T*)>> m_vtable;
	static constexpr std::map<std::string, std::function<void(T*)>> CreateMap()
	{
		std::map<std::string, std::function<void(T*)>> map;
		REGISTER_VFN(map, OnInspectorGUI);
		REGISTER_VFN(map, Update);
		return map;
	}

public:
	void Call(void* instance, std::string) override;
};

#undef REGISTER_VFN
#undef DECL_VFN

template<typename T>
const std::map<std::string, std::function<void(T*)>> ComponentWrapper<T>::m_vtable = ComponentWrapper<T>::CreateMap();

template <typename T>
void ComponentWrapper<T>::Call(void* instance, const std::string fnName)
{
	const auto& fn = m_vtable.find(fnName);
	if (fn != m_vtable.end()) fn->second(static_cast<T*>(instance));
}