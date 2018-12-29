#include "stdafx.h"
#include <process.h> 
#include "HistogramDll.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

HANDLE CreateNewCompletionPort(DWORD dwNumberOfConcurrentThreads)
{
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, dwNumberOfConcurrentThreads);
}

//Objectivo: pr�tica de utiliza��o de IOCP no suporte � execu��o de opera��es ass�ncronas. 
//Para cada quest�o do enunciado deve realizar programas que demonstrem o correto funcionamento.
//A entrega dever� incluir um pequeno relat�rio que explique as decis�es de implementa��o tomadas.

//1) Realize uma biblioteca de suporte a opera��es ass�ncronas sobre ficheiros de texto suportadas em I/O Completion  Port.
//A �nica opera��o a suportar sobre ficheiros de texto corresponde � realiza��o do histograma de todos os
//caracteres do alfabeto presentes no respectivo ficheiro. O histograma corresponde ao somat�rio das ocorr�ncias
//de cada car�cter do alfabeto no ficheiro. A biblioteca deve exportar as seguintes opera��es, pass�veis de serem
//invocadas de forma concorrente(thread-safe).

typedef struct{
	AsyncCallback cb; 
	LPVOID userCtx;

	//std::vector<UINT32>* characters;
	//UINT32* histogram;

	HANDLE hFile_io;
	HANDLE hFile;
	OVERLAPPED overlapped;

} HISTOGRAM_STRUCT, *PHISTOGRAM_STRUCT;

HANDLE hCompletionIO;
static ULONG_PTR completion_key = 0;

volatile DWORD nThreadsBusy;

volatile DWORD initialized = FALSE;
volatile DWORD terminating = FALSE;

static HANDLE terminateEvt;

static UINT EndProcess(DWORD status, PHISTOGRAM_STRUCT histogram, std::vector<UINT32>* characters)
{
	if(histogram)
	{
		CloseHandle(histogram->overlapped.hEvent);

		FlushFileBuffers(histogram->hFile_io);
		//CloseHandle(histogram->hFile_io);
		CloseHandle(histogram->hFile);

		histogram->cb(histogram->userCtx, status, characters);

		free(histogram);
	}

	InterlockedDecrement(&nThreadsBusy);

	if (nThreadsBusy == 0 && terminating)
	{
		SetEvent(terminateEvt);
	}

	return 0;
}

// fun��o executada pelas threads associadas � IOCP
static UINT WINAPI HistogramReadFileAsync(LPVOID arg) {
	
	PHISTOGRAM_STRUCT curHistogram = (PHISTOGRAM_STRUCT)arg;
	LPOVERLAPPED poverlapped = &curHistogram->overlapped;

	DWORD dwError, nBytesRead;
	BYTE bBuffer[BUFFERSIZE];

	std::vector<UINT32> histogram(TNOCHAR);

	while (true)
	{
		ReadFile(curHistogram->hFile, bBuffer, BUFFERSIZE, &nBytesRead, poverlapped);

		switch (dwError = GetLastError())
		{
			case ERROR_HANDLE_EOF:
			{
				// we're reached the end of the file 
				// during the call to ReadFile 

				return EndProcess(SUCCESSFUL_READ_FILE, curHistogram, &histogram);
			}

			case ERROR_IO_PENDING:
			{
				// asynchronous i/o is still in progress 
				// check on the results of the asynchronous read 

				DWORD dwWaitReturn = WaitForSingleObject(curHistogram->overlapped.hEvent, 0);
				if (dwWaitReturn == WAIT_OBJECT_0)
				{
					//bReadDone = GetQueuedCompletionStatus(curHistogram->hCompletionIO, &nBytesRead, &completion_key, &poverlapped, INFINITE);
					if (GetOverlappedResult(curHistogram->hFile, poverlapped, &nBytesRead, TRUE) != 0)
					{
						poverlapped->Offset += nBytesRead;

						for (DWORD n = 0; n <= nBytesRead; n++)
						{
							BYTE ch = bBuffer[n];
							if (ch <= 'z' && ch >= 'a' || ch <= 'Z' && ch >= 'A')
							{
								++histogram[ch];
							}
						}

						continue;
					}
					
					if ((dwError = GetLastError()) == ERROR_HANDLE_EOF)
					{
						return EndProcess(SUCCESSFUL_READ_FILE, curHistogram, &histogram);
					}
					
					return EndProcess(dwError, curHistogram, &histogram);
				}

			} // end case 
			default:
			{
				return EndProcess(dwError, curHistogram, &histogram);
			};

		} // end switch 

	} // end while 
}

//Poss�vel solu��o para ter valor din�mico do n�mero de threads associadas � completion port
//LONG g_nThreadsMin, g_nThreadsMax, g_nThreadsCurrent, g_nThreadsBusy;
//DWORD WINAPI ThreadPoolFunc(PVOID pv) {
//	// Thread is entering pool InterlockedIncrement(&g_nThreadsCurrent); 
//	InterlockedIncrement(&g_nThreadsBusy);
//	for (BOOL bStayInPool = TRUE; bStayInPool;) {
//		// Thread stops executing and waits for something to do LPOVERLAPPED po;
//		InterlockedDecrement(&g_nThreadsBusy);
//		BOOL bOk = GetQueuedCompletionStatus(..., &po, ...); 
//		DWORD dwIOError = GetLastError();
//		int nThreadsBusy = InterlockedIncrement(&g_nThreadsBusy);
//		// Add another thread into the pool? 
//		if (nThreadsBusy == g_nThreadsCurrent) { 
//			if (nThreadsBusy < g_nThreadsMax) { 
//				if (GetCpuUsage() < 75) { 
//					// Add thread to pool 
//					CloseHandle(_beginthreadex(...)); 
//				} 
//			} 
//		}
//		
//		if (!bOk && (dwIOError == WAIT_TIMEOUT)) // Thread timeout
//		// There isn�t much for the server to do, so this thread can die even if there�s
//		// still I/O requests pending 
//			bStayInPool = FALSE;
//	}
//	if (bOk || (po != NULL)) {
//		// Thread woke to process something; process it
//		...
//			if (GetCpuUsage() > 90) { // CPU usage is above 90% if (g_nThreadsCrnt > g_nThreadsMin) // Pool above min
//				bStayInPool = FALSE;
//			}
//	}
//	// Thread is leaving pool InterlockDecrement(&g_nThreadsBusy); InterlockDecrement(&g_nThreadsCurrent); return (0);
//}

//Constr�i a infraestrutura necess�ria ao suporte de opera��es ass�ncronas (histograma) sobre ficheiros de texto.
//Na eventualidade de invoca��o m�ltipla apenas a primeira invoca��o deve ter efeito.
BOOL AsyncInit()
{
	if (InterlockedCompareExchange(&initialized, TRUE, FALSE) == TRUE)
	{
		return FALSE;
	}

	hCompletionIO = CreateNewCompletionPort(MAX_CONCURRENCY);
	assert(hCompletionIO != NULL);

	terminateEvt = CreateEvent(NULL, TRUE, FALSE, NULL);

	return TRUE;
}

//A fun��o HistogramFileAsync inicia o processamento ass�ncrono do histograma do ficheiro de nome file.
//Na conclus�o da opera��o, ou na ocorr�ncia de algum erro durante a sua execu��o, a fun��o de callback(cb) 
//� invocada com o argumento userCtx, o status da opera��o(status) e o histograma calculado (histogram).
//O histograma � constitu�do por uma sequ�ncia de inteiros igual em ordem e n�mero ao alfabeto da l�ngua inglesa.
//Cada entrada da sequ�ncia representa o somat�rio de ocorr�ncias do respectivo car�cter.
//Na aus�ncia de erro o valor de status � zero, caso contr�rio corresponde ao erro dado pela chamada � fun��o GetLastError().
//A fun��o retorna TRUE se a opera��o foi iniciada com sucesso, FALSE caso contr�rio.
BOOL HistogramFileAsync(PCSTR file,	AsyncCallback cb, LPVOID userCtx)
{
	if(!initialized)
	{
		return FALSE;
	}

	PHISTOGRAM_STRUCT Histogram = (PHISTOGRAM_STRUCT)malloc(sizeof(HISTOGRAM_STRUCT));

	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	overlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);
	assert(Histogram->overlapped.hEvent != NULL);
	Histogram->overlapped = overlapped;

	Histogram->hFile = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	assert(Histogram->hFile != INVALID_HANDLE_VALUE);

	Histogram->hFile_io = CreateIoCompletionPort(Histogram->hFile, hCompletionIO, completion_key, 0);
	assert(Histogram->hFile_io != NULL);

	Histogram->cb = cb;
	Histogram->userCtx = userCtx;

	_beginthreadex(NULL, 0, HistogramReadFileAsync, Histogram, 0, NULL);

	InterlockedIncrement(&nThreadsBusy);

	return TRUE;
}

//Termina e destr�i todos os recursos associados � infraestrutura de suporte a opera��es ass�ncronas.
//Na eventualidade de invoca��o m�ltipla, apenas a primeira invoca��o deve ter efeito.
//Valorizam-se solu��es que garantem a execu��o de todas as opera��es de histogramas pendentes
VOID AsyncTerminate()
{
	if(InterlockedCompareExchange(&terminating, TRUE, FALSE) == TRUE)
	{
		return;
	}

	if(nThreadsBusy > 0)
	{
		std::cout << "AsyncTerminate from thread " << GetCurrentThreadId() << " will wait for the termination of "<< nThreadsBusy << " threads!\n\n";

		WaitForSingleObject(terminateEvt, INFINITE);
	}

	CloseHandle(hCompletionIO);
}