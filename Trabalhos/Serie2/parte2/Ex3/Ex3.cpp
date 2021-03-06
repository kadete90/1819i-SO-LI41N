#include "pch.h"
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <crtdbg.h>

#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

void ErrorExit(PTSTR);

int _tmain(int argc, TCHAR *argv[])
{
	SECURITY_ATTRIBUTES saAttr;

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	printf("\n->Start of parent execution.\n");

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit((PTSTR)("StdoutRd CreatePipe"));

	//if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	//	ErrorExit(PTSTR("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit((PTSTR)("Stdin CreatePipe"));

	//if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
	//	ErrorExit(PTSTR("Stdin SetHandleInformation"));

	g_hChildStd_OUT_Wr = GetStdHandle(STD_OUTPUT_HANDLE);
	//g_hChildStd_OUT_Wr = GetStdHandle(STD_INPUT_HANDLE);

	// Create the child process. 

	//LPSTR appName = (LPSTR)"Ex3_ChildProcess.exe";
	LPSTR szCmdline = (LPSTR)"..\\x64\\Debug\\Ex3_ChildProcess.exe";
	//LPSTR szCmdline = (LPSTR)"cmd.exe";
	PROCESS_INFORMATION procInfo;
	SECURITY_ATTRIBUTES sa;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

	siStartInfo.cb = sizeof(STARTUPINFO);

	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;

	siStartInfo.dwX = 100;
	siStartInfo.dwY = 500;
	siStartInfo.dwXSize = 100;
	siStartInfo.dwYSize = 100;

	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL,
		_T(szCmdline),     // command line 
		NULL,//&sa,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_SUSPENDED | CREATE_NEW_CONSOLE,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&procInfo);  // receives PROCESS_INFORMATION 

	//SetStdHandle(STD_OUTPUT_HANDLE, hConOut);

	 // If an error occurs, exit the application. 
	if (!bSuccess)
		ErrorExit((PTSTR)("CreateProcess"));
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		_ASSERT(SetProcessAffinityMask(GetCurrentProcess(), 1) != 0);
		_ASSERT(SetProcessAffinityMask(procInfo.hProcess, 1) != 0);
		_ASSERT(SetThreadAffinityMask(GetCurrentThread(), 1) != 0);
		_ASSERT(SetThreadAffinityMask(procInfo.hThread, 1) != 0);

		_ASSERT(SetThreadPriority(procInfo.hThread, THREAD_PRIORITY_ABOVE_NORMAL) != 0);
		_ASSERT(SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) != 0);

		_ASSERT(SetProcessPriorityBoost(procInfo.hProcess, true) != 0);

		ResumeThread(procInfo.hThread);
	}

	DWORD dwRead, dwWritten;

	CHAR chBuf[BUFSIZE];

	//HANDLE hParentStdIn = GetStdHandle(STD_INPUT_HANDLE);

	//if (hParentStdIn == INVALID_HANDLE_VALUE)
	//	ErrorExit(PTSTR("INVALID hParentStdIn"));

	while(true) {
		bSuccess = ReadFile(GetStdHandle(STD_INPUT_HANDLE), chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		if (chBuf[0] == '+') {
			break;
		}

		bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}

	/*for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}*/

	// Close the pipe handle so the child process stops reading. 
	//if (!CloseHandle(g_hChildStd_IN_Wr))
	//	ErrorExit((PTSTR)("StdInWr CloseHandle"));

	TerminateProcess(procInfo.hProcess, 0);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	//if (!CloseHandle(g_hChildStd_OUT_Rd))
	//	ErrorExit((PTSTR)("StdOutRd CloseHandle"));

	printf("\n->End of parent execution.\n");

	// Wait until child process exits
	//WaitForSingleObject(piProcInfo.hProcess, INFINITE);

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 

	return 0;
}

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}