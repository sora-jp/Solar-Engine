#pragma once

#if SOLAR_ENGINE_BUILD
	#define SOLAR_API __declspec(dllexport)
	#define ENTT_API_EXPORT
#else
	#define SOLAR_API __declspec(dllimport)
	#define ENTT_API_IMPORT
#endif

#if SOLAR_SUBSYSTEM_BUILD
#define SUBSYSTEM_API __declspec(dllexport)
#else
#define SUBSYSTEM_API
#endif

#include <memory>
#include <functional>

template<typename T> using Shared = SOLAR_API std::shared_ptr<T>;
template<typename T> using Unique = SOLAR_API std::unique_ptr<T>;
template<typename T> using Weak   = SOLAR_API std::weak_ptr  <T>;

template<typename T, typename ...Args> Shared<T> MakeShared(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename ...Args> Unique<T> MakeUnique(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

#define  ITERATOR(collection) std::begin(collection),  std::end(collection)
#define RITERATOR(collection) std::rbegin(collection), std::rend(collection)

template<class It, class Fn>
void foreach_impl(It begin, It end, Fn&& func)
{
	static_assert(!std::is_void_v<It>, "Attempting to iterate on object without iterators is retarded bro");
	static_assert(std::is_invocable_v<Fn, decltype(*std::declval<It>())>, "Invalid argument to iteration function");
	for (It iter = begin; iter != end; ++iter) std::invoke(std::forward<Fn>(func), std::forward<decltype(*iter)>(*iter));
}

#define ITERTYPE(x) decltype(*std::declval<x>().begin())
#define ENABLE_IF_ITERCALL(coll, func) std::enable_if_t<std::is_invocable_v<func, ITERTYPE(coll)>, bool> = true

template<class Coll, class Fn, ENABLE_IF_ITERCALL(Coll, Fn)> void foreach(Coll collection, Fn func)
{
	//static_assert(!std::is_void_v<decltype(std::declval<Coll>().begin())>);
	//static_assert(!std::is_void_v<decltype(std::declval<Coll>().end())>);

	//for (auto& iter = collection.begin(); iter != collection.end(); ++iter) 
	//	std::invoke(std::forward<Fn>(func), std::forward<decltype(*iter)>(*iter));
	foreach_impl(collection.begin(), collection.end(), std::forward<Fn>(func));
}

template<class Coll, class Fn, ENABLE_IF_ITERCALL(Coll, Fn)> void foreach_reverse(Coll collection, Fn func)
{
	//static_assert(!std::is_void_v<decltype(std::declval<Coll>().rbegin())>);
	//static_assert(!std::is_void_v<decltype(std::declval<Coll>().rend())>);

	//for (auto& iter = collection.rbegin(); iter != collection.rend(); ++iter) 
	//	std::invoke(std::forward<Fn>(func), std::forward<decltype(*iter)>(*iter));
	foreach_impl(collection.rbegin(), collection.rend(), std::forward<Fn>(func));
}

template<class Coll, class Fn, std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Fn, decltype(*std::declval<Coll>().begin())>, bool>, bool> = true> bool any(Coll collection, Fn func)
{
	static_assert(!std::is_void_v<decltype(std::declval<Coll>().begin())>);
	static_assert(!std::is_void_v<decltype(std::declval<Coll>().end())>);

	for (auto& iter = collection.begin(); iter != collection.end(); ++iter) 
		if (std::invoke(std::forward<Fn>(func), std::forward<decltype(*iter)>(*iter))) return true;
	
	return false;
}