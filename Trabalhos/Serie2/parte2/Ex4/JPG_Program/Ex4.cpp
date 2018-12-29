#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "../Include/JPEGExifUtils.h"
#include "../Include/PrintUtils.h"
#include "../Include/List.h"
#include <string>

DWORD getNumberOfProcessors() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

// Auxiliary node to support result
typedef struct {
	LIST_ENTRY link;
	PCHAR fileToDelete;
} FILE_NODE, *PFILE_NODE;

// Struct to hold result of operation
typedef struct {
	DWORD filesCnt;
	DWORD dirCnt;
	LIST_ENTRY filesToDeleteCollection;
} RESULT, *PRESULT;

// Struct to hold context to JPG_ProcessExifTags
typedef struct {
	PCSTR fileName;
	PCSTR filepathSrc;
	PCSTR pathDst;
	PRESULT res;
} JPG_CTX, *PJPG_CTX;

// The synchronization object.
// Usually, this would be passed to the worker thread as part of
// context data but to simplify the example I put it here.
HANDLE g_syncEventHandle = 0;

typedef struct {
	PCHAR filePath;
	JPG_CTX jpgCtx;
} THREAD_CTX, *PTHREAD_CTX;

// Callback to JPG_ProcessExifTags
BOOL ProcessExifTag(LPCVOID ctx, DWORD tag, LPVOID value) {
	BOOL retVal = true;
	if (tag == 0x132 || tag == 0x9003 || tag == 0x9004) {
		printf("Tag [%xH] --> \"%s\"\n", tag, (CHAR*)value);
		PCHAR nextTk;
		PCHAR year = strtok_s((PCHAR)value, ":", &nextTk);
		PCHAR month = strtok_s(NULL, ":", &nextTk);
		PCHAR day = strtok_s(NULL, " ", &nextTk);
		PJPG_CTX pctx = (PJPG_CTX)ctx;
		CHAR filepath[MAX_PATH];
		sprintf_s(filepath, "%s/%s_%s_%s", pctx->pathDst, year, month, day);
		BOOL ret = CreateDirectoryA(filepath, NULL);
		pctx->res->dirCnt += ret == TRUE;
		sprintf_s(filepath, "%s/%s", filepath, pctx->fileName);
		ret = CopyFileA(pctx->filepathSrc, filepath, TRUE);
		pctx->res->filesCnt += ret == TRUE;
		if (ret == TRUE) {
			// The file will be deleted only if it is copied.
			PFILE_NODE newNode = (PFILE_NODE)malloc(sizeof(FILE_NODE));
			newNode->fileToDelete = _strdup(pctx->filepathSrc);
			InsertTailList(&pctx->res->filesToDeleteCollection, &newNode->link);
		}
		retVal = false;
	}
	return retVal;
}

volatile BOOL checkedFilesToProcess = FALSE;
volatile LONG nrWorkItems = 0;

DWORD WINAPI ThreadProc(LPVOID arg){

	PTHREAD_CTX ctx = PTHREAD_CTX(arg);

	PCHAR filePath = ctx->filePath;

	// Process file archive
	printf("ThreadId %lu, Processing %s\n", GetCurrentThreadId(), filePath);

	if(JPEG_ProcessExifTagsA(filePath, ProcessExifTag, &ctx->jpgCtx))
	{
		printf_s("----> ThreadId %lu, Processed: %s \n", GetCurrentThreadId(), filePath);
	}
	else
	{
		printf_s("----> ThreadId %lu, Error processing: %s \n", GetCurrentThreadId(), filePath);
	}

	//EnterCriticalSection(pCtx->cs);

	if(InterlockedDecrement(&nrWorkItems) == 0 && checkedFilesToProcess)
	{
		SetEvent(g_syncEventHandle);
	}

	//LeaveCriticalSection(pCtx->cs);
	
	return 0;
}

VOID OrganizePhotosByDateTaken(PCHAR srcPath, PCHAR dstPath, PRESULT res) {

	CHAR buffer[MAX_PATH];		// auxiliary buffer
								// the buffer is needed to define a match string that guarantees 
								// a priori selection for all files
	sprintf_s(buffer, "%s/%s", srcPath, "*.*");

	WIN32_FIND_DATAA fileData;
	HANDLE fileIt = FindFirstFileA(buffer, &fileData);
	if (fileIt == INVALID_HANDLE_VALUE) return;

	// Process directory entries
	do {
		CHAR filePath[MAX_PATH];
		sprintf_s(filePath, "%s/%s", srcPath, fileData.cFileName);

		if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			// Not processing "." and ".." files!
			if (strcmp(fileData.cFileName, ".") && strcmp(fileData.cFileName, "..")) {
				// Recursively process child directory
				OrganizePhotosByDateTaken(filePath, dstPath, res);
			}
		}
		else {

			PTHREAD_CTX ctx = (PTHREAD_CTX)malloc(sizeof(THREAD_CTX));

			// better way to save this values?
			auto innerPath = new char[MAX_PATH];
			strncpy_s(innerPath, MAX_PATH, filePath, MAX_PATH);
			ctx->filePath = innerPath;

			auto innerFileName = new char[MAX_PATH];
			strncpy_s(innerFileName, MAX_PATH, fileData.cFileName, MAX_PATH);

			ctx->jpgCtx = { innerFileName, innerPath, dstPath, res };

			if(QueueUserWorkItem(ThreadProc, (PVOID)ctx, WT_EXECUTEDEFAULT))
			{
				InterlockedIncrement(&nrWorkItems);
			}
		}
	} while (FindNextFileA(fileIt, &fileData) == TRUE);

	FindClose(fileIt);
}

DWORD _tmain(DWORD argc, PCHAR argv[]) {

	if (argc < 4) {
		printf("Use: %s <repository path src> <repository path dst> -D|K", argv[0]);
		exit(0);
	}

	printf("Main ThreadId %lu\n", GetCurrentThreadId());

	// Initiate arguments to operation
	RESULT res = { 0 };
	InitializeListHead(&res.filesToDeleteCollection);

	g_syncEventHandle = CreateEvent(0, TRUE, FALSE, 0);

	// Realize operation
	OrganizePhotosByDateTaken(argv[1], argv[2], &res);

	checkedFilesToProcess = TRUE;

	if(nrWorkItems > 0)
	{
		WaitForSingleObject(g_syncEventHandle, INFINITE);
	}

	// And remember to clean up
	CloseHandle(g_syncEventHandle);

	// Print result and delete files copied
	BOOL toDelete = *(argv[3] + 1) == 'D';
	while (IsListEmpty(&res.filesToDeleteCollection) == FALSE) {
		PFILE_NODE fileNode = CONTAINING_RECORD(RemoveHeadList(&res.filesToDeleteCollection), FILE_NODE, link);
		if (toDelete) {
			BOOL ret = DeleteFileA(fileNode->fileToDelete);
			if (ret == FALSE)
				PrintLastError();
		}
		free(fileNode->fileToDelete);
		free(fileNode);
	}
	printf("Directories created = %d\nFiles copied/deleted = %d\n", res.dirCnt, res.filesCnt);

	PRESS_TO_FINISH(_T(""));

	return 0;
}
