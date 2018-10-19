#pragma once
#include <sal.h>
#include <Windows.h>

typedef struct {
	DWORD img;
	DWORD map;
	DWORD prv;
} CommitCounters, *PCommitCounters;

bool GetCommitCountersFromProcess(int pid, _Out_ PCommitCounters counters);

