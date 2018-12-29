#include "pch.h"
#include <windows.h>
#include <iostream>
//#include "../Include/HistogramDll.h"
#include <vector>
#include "../SO_SE3/HistogramDll.h"

CRITICAL_SECTION cs;

VOID ExecuteCallback(LPVOID userCtx, DWORD status, std::vector<UINT32>*  histogram)
{
	EnterCriticalSection(&cs);

	std::cout << "Executing callback on worker thread " << GetCurrentThreadId() << "\nHistogram of file " << LPCTSTR(userCtx) << "!\n";

	UINT32 totalEnChars = 0;

	if(status == SUCCESSFUL_READ_FILE)
	{
		std::vector<UINT32> chars = *histogram;

		for (int i = 0; i < TNOCHAR; ++i)
		{
			if (chars[i] > 0)
			{
				totalEnChars += chars[i];

				std::cout << (char)i << " ( " << chars[i] << " )\n";
			}
		}
		std::cout << "Total filtered chars: "<< totalEnChars<<"\n\n";
	}
	else
	{
		std::cout << "!!! Error reading this file, GetLastError() =" << status << "!\n";
	}

	LeaveCriticalSection(&cs);
}

//2)[Opcional] Com base na infra - estrutura anterior, realize a função BOOL PrintHistogramFolder(PCSTR folder),
//que escreve no stdout o histograma para cada ficheiro presente na pasta de nome folder.A anteceder a escrita de um
//histograma deve ser escrito o nome do ficheiro correspondente.
//A produção do histograma é realizado através da função HistogramFileAsync desenvolvida no ponto anterior.
//A função PrintHistogramFolder é síncrona, retornando apenas quando todos os histogramas forem escritos no stdout
//(retornando TRUE), ou for encontrado algum erro(retornando FALSE).
//BOOL PrintHistogramFolder(PCSTR folder)
//{
//	
//}

int main()
{
	std::cout << "Hello World from Main thread " << GetCurrentThreadId() << "!\n\n";

	InitializeCriticalSection(&cs);

	AsyncInit();
	AsyncInit();

	auto file1 = TEXT("..\\toRead.txt");
	auto file2 = TEXT("..\\toRead2.txt");

	if(HistogramFileAsync(file1, ExecuteCallback, (LPVOID)file1))
	{
		std::cout << "Initialized HistogramFileAsync for " << file1 << " with success!\n";
	}
	else
	{
		std::cout << "Error initializing HistogramFileAsync for " << file1 << "\n";
	}

	if (HistogramFileAsync(file2, ExecuteCallback, (LPVOID)file2))
	{
		std::cout << "Initialized HistogramFileAsync for " << file2 << " with success!\n";
	}
	else
	{
		std::cout << "Error initializing HistogramFileAsync for " << file2 << "\n";
	}

	std::cout << "\n";

	AsyncTerminate();
	AsyncTerminate();

	std::cout << "Main thread will shutdown!\n";
}