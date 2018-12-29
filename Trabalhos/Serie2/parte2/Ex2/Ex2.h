#pragma once
#include <Windows.h>

#ifndef EX2_H
#define EX2_H

#pragma data_seg(".mysegmentname") 

#define BLOCK_SIZE 1000

LONG volatile COUNT = 0;
UINT NUMBER_OF_ITERATIONS = 1000 * 1000;
#pragma data_seg()
#pragma comment(linker, "/section:.mysegmentname,rws")

static UINT WINAPI ThreadProc(LPVOID arg)
{
	while (COUNT < NUMBER_OF_ITERATIONS)
	{
		InterlockedIncrement(&COUNT);
		//printf("\nThreadId: %i, count: %i", GetCurrentThreadId(), COUNT);
		Sleep(0); // To force context switch
	}

	return 0;
}

#endif
