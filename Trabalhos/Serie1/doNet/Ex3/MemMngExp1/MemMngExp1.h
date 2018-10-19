#pragma once

#ifdef MEM_MANAGMENT_EXP1_EXPORT
#define MEM_MANAGMENT_EXP1_API __declspec(dllexport)
#else
#define MEM_MANAGMENT_EXP1_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	MEM_MANAGMENT_EXP1_API void Experience1(); // Test shared data

#ifdef __cplusplus
}
#endif
