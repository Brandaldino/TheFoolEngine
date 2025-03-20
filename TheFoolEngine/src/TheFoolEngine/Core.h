#pragma once


#ifdef TF_PLATFORM_WINDOWS
	#ifdef TF_BUILD_DLL
		#define TheFoolEngine_API __declspec(dllexport)
	#else 
		#define TheFoolEngine_API __declspec(dllimport)
	#endif // TF_BUILD_DLL
#else
	#error TheFoolEngine only support Windows!
#endif // TF_PLATFORM_WINDOWS
