#include "pch.h"
#include "CommitCounters.h"
#include "Psapi.h"
#include <iostream>
#include "CalculateWSPrivate.h"

// Implemente a fun��o GetCommitCountersFromProcess que determina a dimens�o da mem�ria Committed
// alocada ao processo com o identificador pid descriminada pelos tipos de aloca��o (imagem, mapeada ou privada)
// A fun��o retorna TRUE e em counters os valores dos contadores de Commit ou FALSE e zero nos contadores, em caso de erro. 

bool GetCommitCountersFromProcess(int pid, _Out_ PCommitCounters counters)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	if (nullptr == hProcess)
	{
		std::cout << "Unable to open the process : " << GetLastError() << std::endl;
		return false;
	}

	MEMORY_BASIC_INFORMATION lpBuffer;
	LPVOID lpAddress = nullptr, lpInicio, lpFim;
	
	__try
	{
		while(VirtualQueryEx(hProcess, lpAddress, &lpBuffer, sizeof lpBuffer)){
			lpInicio = lpAddress;
			lpFim = PBYTE(lpInicio) + lpBuffer.RegionSize;
			lpAddress = lpFim;

			if(lpBuffer.State != MEM_COMMIT)
			{
				continue;
			}

			switch (lpBuffer.Type)
			{
				case MEM_IMAGE:
				{
					counters->img += lpBuffer.RegionSize / 1024;
					break;
				}
				case MEM_MAPPED:
				{
					counters->map += lpBuffer.RegionSize / 1024;
					break;
				}
				case MEM_PRIVATE:
				{
					counters->prv += lpBuffer.RegionSize / 1024;
					break;
				}
				
				default:
					break;
			}
		}
	}
	__finally
	{
		CloseHandle(hProcess);
	}

	return true;
}