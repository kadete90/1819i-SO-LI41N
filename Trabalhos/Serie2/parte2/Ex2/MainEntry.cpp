// Ex2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>
#include "Ex2.h"

//Escreva programas para determinar o tempo de comutação de threads no sistema operativo Windows.
//Teste o tempo de comutação entre threads do mesmo processo.

static DWORD runSameProcess()
{
	printf("Ex2 - Running %i context switch's in the same process\n-----", NUMBER_OF_ITERATIONS);

	COUNT = 0;

	const auto thread_a = HANDLE(_beginthreadex(nullptr, 0, ThreadProc, nullptr, CREATE_SUSPENDED, nullptr));
	const auto thread_b = HANDLE(_beginthreadex(nullptr, 0, ThreadProc, nullptr, CREATE_SUSPENDED, nullptr));

	SetThreadAffinityMask(thread_a, 1);
	SetThreadAffinityMask(thread_b, 1);

	SetThreadPriority(thread_a, THREAD_PRIORITY_HIGHEST);
	SetThreadPriority(thread_b, THREAD_PRIORITY_HIGHEST);

	DWORD ticks = GetTickCount();

	ResumeThread(thread_a);
	ResumeThread(thread_b);

	WaitForSingleObject(thread_a, INFINITE);
	WaitForSingleObject(thread_b, INFINITE);

	ticks = GetTickCount() - ticks;

	printf("\nAverage context switch took %.6f milliseconds", static_cast<float>(ticks) / NUMBER_OF_ITERATIONS);

	CloseHandle(thread_a);
	CloseHandle(thread_b);

	return 0;
}

static DWORD runDifferentProcess()
{
	printf("Ex2 - Running %i context switch's in different processes\n-----", NUMBER_OF_ITERATIONS);

	COUNT = 0;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof si);
	si.cb = sizeof si;
	ZeroMemory(&pi, sizeof pi);

	LPSTR app_name = LPSTR("DLL_Process.exe");

	if (CreateProcess(nullptr, app_name, nullptr, nullptr, TRUE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi))
	{
		const auto thread_a = HANDLE(_beginthreadex(nullptr, 0, ThreadProc, nullptr, CREATE_SUSPENDED, nullptr));

		SetThreadAffinityMask(thread_a, 1);
		SetThreadAffinityMask(pi.hThread, 1);

		SetThreadPriority(thread_a, THREAD_PRIORITY_HIGHEST);
		SetThreadPriority(pi.hThread, THREAD_PRIORITY_HIGHEST);

		DWORD ticks = GetTickCount();

		ResumeThread(thread_a);
		ResumeThread(pi.hThread);

		CloseHandle(pi.hThread);

		WaitForSingleObject(thread_a, INFINITE);
		WaitForSingleObject(pi.hProcess, INFINITE);

		ticks = GetTickCount() - ticks;
				
		printf("\nAverage context switch took %.6f milliseconds", static_cast<float>(ticks) / NUMBER_OF_ITERATIONS);

		CloseHandle(thread_a);		
			
		DWORD dwExitCode;
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		CloseHandle(pi.hProcess);
	}
	else
	{
		printf("\nError Creating a Process: %i", GetLastError());
	}
	
	return 0;
}

DWORD _tmain()
{
	printf("Ex2 - Testing context switch duration time\n-----");
	printf("\n1 for Same Process test");
	printf("\n2 for different Process test");

	char _char = 0;

	while (true)
	{
		if(_char != '\n')
		{
			printf("\n-----\n");
		}

		_char = getchar();
		if(_char == '1')
		{			
			runSameProcess();
		}
		else if(_char == '2')
		{
			runDifferentProcess();
		}
	}
}