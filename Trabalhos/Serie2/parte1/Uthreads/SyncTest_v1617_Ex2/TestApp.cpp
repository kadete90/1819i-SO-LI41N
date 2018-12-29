#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "UThread.h"
#include "List.h"
#include "USynch.h"

int val;
Event event;

VOID ThreadFunc(UT_ARGUMENT arg) {
	INT id = (INT)arg;
	printf("Start T%d\n", id);
	if (++val % 2 == 1) {
		EventWait(&event);
		printf("After Wait T%d\n", id);
	}
	if (val > 1) EventSet(&event);
	printf("End T%d\n", id);
}
DWORD _tmain(DWORD argc, PTCHAR argv[]) {
	UtInit();

	EventInit(&event, false);
	for (int i = 1; i <= 3; i++)
		UtCreate(ThreadFunc, (UT_ARGUMENT)i);

	UtRun();
	UtEnd();

	printf("Press enter key to finish...\n");
	getchar();

	return 0;
}