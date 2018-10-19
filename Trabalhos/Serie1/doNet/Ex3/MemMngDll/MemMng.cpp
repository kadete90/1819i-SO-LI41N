#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "MemMng.h"
#include "../MemMngApp/PrintUtils.h"

/*
 * Size of physical pages in this system. 
 * It is initiated when this DLL is loaded into process.
 */
static DWORD pageSz;

/*
 * Entry point of DLL. 
 * Called whenever this DLL is loaded or unloaded into a process or 
 * one thread is created or terminated in the process context where the DLL is loaded.
 */
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		pageSz = si.dwPageSize;
	}
	return true;
}

/*
 * Private data on memory managment
 */
void Experience1() {
	PRESS_TO_GO(_T("Ponto 1"));

	HMODULE lib = LoadLibrary(_T("MemMngExp1.dll"));
	if (lib == 0) {
		printf("Exp1: cannot load library. Error #%d\n", GetLastError());
		return;
	}

	VOID(*exp1)() = (VOID(*)())GetProcAddress(lib, "Experience1");
	if (exp1 == 0) {
		printf("Exp1: cannot get function address. Error #%d\n", GetLastError());
		return;
	}

	exp1();

	FreeLibrary(lib);
	PRESS_TO_GO(_T("Ponto 5"));
}

/*
 * VirtualAlloc on memory managment
 */
void Experience2() {
	PRESS_TO_GO(_T("Ponto 1"));
	LPVOID base = VirtualAlloc(0, 256 * MEGA, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (base == NULL) {
		printf("Exp2: cannot execute experience. Error #%d\n", GetLastError());
		return;
	}
	PRESS_TO_GO(_T("Ponto 2"));
	touch_read(base, 1024 * 32, pageSz);
	PRESS_TO_GO(_T("Ponto 3"));
	VirtualFree(base, 128 * MEGA, MEM_DECOMMIT);
	PRESS_TO_GO(_T("Ponto 4"));
	VirtualFree(base, 0, MEM_RELEASE);
	PRESS_TO_GO(_T("Ponto 5"));
}

/*
 * FileMapping on memory managment
 */
void Experience3() {
	HANDLE hBigFile = CreateFile(_T("BigFile.bin"), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hBigFile == INVALID_HANDLE_VALUE) {
		printf("exp3: cannot create big file. Error #%d\n", GetLastError());
		return;
	}
	DWORD e = SetFilePointer(hBigFile, 0x10000000-1, 0, FILE_BEGIN);
	if (e == INVALID_SET_FILE_POINTER) {
		printf("exp3: cannot set file pointer. Error #%d\n", GetLastError());
		return;
	}
	e = WriteFile(hBigFile, "1", 1, NULL, NULL);
	if (e == false) {
		printf("exp3: cannot write file. Error #%d\n", GetLastError());
	}
	CloseHandle(hBigFile);
		
	PRESS_TO_GO(_T("Ponto 1"));
	HANDLE hFile = CreateFile(_T("BigFile.bin") , GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Exp3: cannot create file. Error #%d\n", GetLastError());
		return;
	}
	HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, 0);
	if (hMapFile == NULL) {
		printf("Exp3: cannot create section object. Error #%d\n", GetLastError());
		return;
	}
	LPVOID base = MapViewOfFile(hMapFile, FILE_READ_ACCESS, 0, 0, 0);
	if (base == NULL) {
		printf("Exp3: cannot map view of file. Error #%d\n", GetLastError());
		return;
	}
	PRESS_TO_GO(_T("Ponto 2"));
	CloseHandle(hMapFile);
	CloseHandle(hFile);
	touch_read(base, 1024 * 32, pageSz);
	PRESS_TO_GO(_T("Ponto 3"));
	UnmapViewOfFile(base);
	PRESS_TO_GO(_T("Ponto 4"));
	DeleteFile(_T("BigFile.bin"));
	PRESS_TO_GO(_T("Ponto 5"));
}

