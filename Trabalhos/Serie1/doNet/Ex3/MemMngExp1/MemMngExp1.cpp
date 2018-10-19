#include <Windows.h>
#include "MemMngExp1.h"
#include "../MemMngApp/PrintUtils.h"

/*
* Private data on memory managment
*/
void Experience1() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD pageSz = si.dwPageSize;

	// Static initialized data
	static BYTE big_data[256 * MEGA] = { 1, 2, 3 };
	PRESS_TO_GO(_T("Ponto 2"));

	// Reads 32K page frames of static data
	touch_read(big_data, 1024 * 32, pageSz);
	PRESS_TO_GO(_T("Ponto 3"));

	// Writes same 32K blocks
	touch_write(big_data, 1024 * 32, pageSz);
	PRESS_TO_GO(_T("Ponto 4"));
}
