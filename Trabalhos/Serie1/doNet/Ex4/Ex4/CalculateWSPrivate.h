#pragma once
#include <Windows.h>

class CalculateWSPrivate
{
public:
	CalculateWSPrivate();
	~CalculateWSPrivate();
};

DWORD CalculateWSPrivate(HANDLE hProcess, DWORD processID);