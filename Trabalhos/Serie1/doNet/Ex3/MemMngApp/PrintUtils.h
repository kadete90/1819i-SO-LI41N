#pragma once

#include <tchar.h>
#include <stdio.h>

#define MEGA (1024*1024)
#define GIGA (MEGA * 1024)

#define PRESS_TO_GO(s) do {							\
	_tprintf(s _T(": Press any key to continue "));	\
	TCHAR c = _gettchar();							\
	if (c != '\n') _puttchar(_T('\n'));				\
} while (0)

/*
* Touchs in numberOfPages pages with read accesses starting at address base.
*/
static void touch_read(LPVOID base, DWORD numberOfPages, DWORD pageSz) {
	DWORD sum = 0;
	PCHAR b = (PCHAR)base;
	for (DWORD i = 0; i < numberOfPages; i++) {
		sum += b[i * pageSz];
	}
}
/*
* Touchs in numberOfPages pages with write accesses starting at address base.
* Writes only into first byte of each page with 1.
*/
static void touch_write(LPVOID base, DWORD numberOfPages, DWORD pageSz) {
	PCHAR b = (PCHAR)base;
	for (DWORD i = 0; i < numberOfPages; i++) {
		b[i * pageSz] = 1;
	}
}

