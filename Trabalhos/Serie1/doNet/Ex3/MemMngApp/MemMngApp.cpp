#include <Windows.h>
//#include <stdio.h> // deprecated
#include <cstdio>
//#include "MemMng.h"
#include "../MemMngDll/MemMng.h"

#define A2NUMBER(n) ((*(n)) & 0xF)


int main(DWORD argc, PCHAR argv[]) {
	DWORD number = 0;
	DWORD pid = 0;
	// Array of experiences
	void(*exps[])() = {Experience1, Experience2, Experience3};
	if (argc < 2) goto error;

	pid = GetCurrentProcessId();
	printf("Current process id #%d [0x%x]\n", pid, pid);
	printf("Executing Experience%d\n", A2NUMBER(argv[1]));

	number = A2NUMBER(argv[1]);
	number -= 1;
	if (number > 2) goto error;
	exps[number]();

	return 0;

error:
	printf("Use> %s <experience number>\n     <experience number>: 1, 2 ou 3\n", argv[0]);
	return 0;

}