#pragma once

#ifdef HISTOGRAMDLL_EXPORTS
#define HISTOGRAM_API __declspec(dllexport)
#else
#define HISTOGRAM_API __declspec(dllimport)
#endif

#define TNOCHAR 128  /* Total Number of characters is 128: 0 - 127 */

#define BUFFERSIZE 4096		/* I/O buffer size used during server request processing */

// é usado o nível de concorrencia por omissao
#define MAX_CONCURRENCY 0

DWORD SUCCEEDED = 0;

typedef VOID(*AsyncCallback)(LPVOID userCtx, DWORD status, UINT32 * histogram);

#ifdef __cplusplus
extern "C" {
#endif
	HISTOGRAM_API BOOL AsyncInit();

	HISTOGRAM_API BOOL HistogramFileAsync(PCSTR file, AsyncCallback cb, LPVOID userCtx);

	HISTOGRAM_API VOID AsyncTerminate();
#ifdef __cplusplus
}
#endif


