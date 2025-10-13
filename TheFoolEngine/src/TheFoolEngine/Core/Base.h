#pragma once

#include <memory>

#ifdef TF_PLATFORM_WINDOWS
	#define THEFOOLENGINE_API
#else
	#error TheFoolEngine only support Windows!
#endif // TF_PLATFORM_WINDOWS

#ifdef TF_DEBUG
	#define TF_ENABLE_ASSERTS
#endif

#ifdef TF_ENABLE_ASSERTS
	#define TF_ASSERT(x,...) {if(!(x)){TF_ERROR("Assertion Failed: {0}",__VA_ARGS__); __debugbreak();}}
	#define TF_CORE_ASSERT(x,...){if(!(x)){TF_CORE_ERROR("Assertion Failed: {0}",__VA_ARGS__); __debugbreak();}}
#else
	#define TF_ASSERT(x,...)
	#define TF_CORE_ASSERT(x,...)
#endif // ASSERTS


#define BIT(x)(1 << x)

#define TF_BIND_EVENT_FN(fn) std::bind(&fn,this,std::placeholders::_1)


namespace TheFoolEngine{

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) 
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) 
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}
