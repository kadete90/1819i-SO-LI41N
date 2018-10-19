#pragma once

#ifdef MEM_MANAGMENT_EXPORT
#define MEM_MANAGMENT_API __declspec(dllexport)
#else
#define MEM_MANAGMENT_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	MEM_MANAGMENT_API void Experience1(); // Test shared data
	MEM_MANAGMENT_API void Experience2(); // Test use of VirtualAlloc
	MEM_MANAGMENT_API void Experience3(); // Test use of FileMapping

#ifdef __cplusplus
}
#endif
