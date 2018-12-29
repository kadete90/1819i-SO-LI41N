#pragma once
#include <Windows.h>
#include <vector>

#ifdef HISTOGRAMDLL_EXPORTS
#define HISTOGRAM_API __declspec(dllexport)
#else
#define HISTOGRAM_API __declspec(dllimport)
#endif

#define TNOCHAR 128  /* Total Number of characters is 128: 0 - 127 */

#define BUFFERSIZE 1024 /* I/O buffer size used during request processing */

#define MAX_CONCURRENCY 0 // default value

DWORD SUCCESSFUL_READ_FILE = 0;

//typedef VOID(*AsyncCallback)(LPVOID userCtx, DWORD status, UINT32 * histogram);
typedef VOID(*AsyncCallback)(LPVOID userCtx, DWORD status, std::vector<UINT32>* histogram);

#ifdef __cplusplus
extern "C" {
#endif
	HISTOGRAM_API BOOL AsyncInit();

	HISTOGRAM_API BOOL HistogramFileAsync(PCSTR file, AsyncCallback cb, LPVOID userCtx);

	HISTOGRAM_API VOID AsyncTerminate();
#ifdef __cplusplus
}
#endif


