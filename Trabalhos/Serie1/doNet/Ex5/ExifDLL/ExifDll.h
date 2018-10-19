#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXIFDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXIF_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#include <Windows.h>

#ifdef EXIFDLL_EXPORTS
#define EXIF_API __declspec(dllexport)
#else
#define EXIF_API __declspec(dllimport)
#endif

class EXIF_API ExifApi {
public:
	void PrintExifTags(TCHAR* file);
};

#ifdef __cplusplus
extern "C" {
#endif
	EXIF_API void PrintExifTags(TCHAR* file);
#ifdef __cplusplus
}
#endif