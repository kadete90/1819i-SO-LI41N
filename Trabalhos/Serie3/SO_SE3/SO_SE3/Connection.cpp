#include "stdafx.h"
#include "windows.h"
#include <process.h>
#include "Connection.h"
#include <cwchar>
#include <cstdio>
#include <stdlib.h>

HANDLE CreateCompletionPort(int maxConcurrency) {
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency);
}

BOOL CompletionPortAssociateHandle(HANDLE cpHandle, HANDLE devHandle, ULONG_PTR completionKey) {
	HANDLE h = CreateIoCompletionPort(devHandle, cpHandle, completionKey, 0);
	return h == cpHandle;
}

// função executada pelas threads associadas à IOCP
static UINT WINAPI ProcessOpers(LPVOID arg) {
	HANDLE completionPort = (HANDLE)arg;
	DWORD transferedBytes;
	CONNECTION *c;
	OVERLAPPED *ovl;

	BOOL response;

	//printf("start worker!\n");
	while (TRUE) {
		if (!(response = GetQueuedCompletionStatus(completionPort, &transferedBytes,
			(PULONG_PTR)&c, &ovl, INFINITE)) &&
			GetLastError() != ERROR_HANDLE_EOF) {
			printf_s("Error %d getting activity packet!\n", WSAGetLastError());
			return 0;
		}

		switch (c->activityState) {

		case RECV_REQUEST:
			// _tprintf(_T("Recv request done from socket %p, transfered bytes=%d\n"), 
			//	(char*)activity->connectionSocket, transferedBytes);
			if (transferedBytes == 0) {
				// connection closed by client
				ConnectionDestroy(c);
			}
			else {
				// in this case we assume the command was get completely
				// in one transfer
				//assert(transferedBytes == sizeof(OperationRequest));

				c->len = transferedBytes;

				ProcessCommand(c);
			}
			break;
		case SEND_RESPONSE:
			//_tprintf(_T("Send response done from socket %p\n"), activity->connectionSocket);
			// a response was completed, start a new receive command
			// in this case we assume the response was sent completely
			// in one transfer
			//assert(transferedBytes == sizeof(OperationResponse));

			ProcessAnswerCompletion(c);

			break;
		}

	}
}

// associa um conjunto de threads à IOCP
VOID CreateThreadPool(HANDLE ioPort) {
	for (int i = 0; i < MAX_THREADS; ++i) {
		_beginthreadex(NULL, 0, ProcessOpers, (LPVOID)ioPort, 0, NULL);
	}
}
/*
* Used on socket buffered I/O
* fill receive buffer
*/
void AsyncRead(PCONNECTION c) {
	int err = 0;
	WSABUF DataBuf;
	DataBuf.len = BUFFERSIZE;
	DataBuf.buf = c->bufferIn;
	DWORD flags = 0;
	DWORD x, RecvBytes = 0;

	x = WSARecv(c->socket,&DataBuf, 1, &RecvBytes, &flags, &c->ol, NULL);
	if ((x == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
		wprintf(L"WSARecv failed with error: %d\n", err);
		c->len = -1;
		return;
	}
	
	c->rPos = 0;

}



/*
* Used on socket buffered I/O
* flush send buffer to socket
*/
void ConnectionFlushBufferToSocket(PCONNECTION c) {
	
	int err = 0;
	WSABUF DataBuf;
	DataBuf.len = c->wPos;
	DataBuf.buf = c->bufferOut;
	DWORD flags = 0;
	if ((WSASend(c->socket, &DataBuf, 1, NULL, flags, &c->ol, NULL) == SOCKET_ERROR) &&
		(WSA_IO_PENDING != (err = WSAGetLastError()))) {
		printf("WSASend failed with error: %d\n", err);
	}
	//send(c->socket, c->bufferOut, c->wPos, 0);
	c->wPos = 0;
	
}

void FlushSync(PCONNECTION c) {

	send(c->socket, c->bufferOut, c->wPos, 0);
	c->wPos = 0;

}

/*
* reads a line (terminate with \r\n pair) from the connection socket
* using the buffer the I/O buffer in PHttpConnection "cn"
*
* Return the number of line readed
*/
int ConnectionGetLine(PCONNECTION cn, char *buffer, int bufferSize) {
	int i = 0;
	int c;

	while (i < bufferSize - 1 && (c = CGetChar(cn)) != -1 && c != '\r')
		buffer[i++] = c;
	if (c == -1)
		return -1;
	if (c == '\r')
		c = CGetChar(cn); /* read line feed */
	buffer[i] = 0;
	return i;
}


/*
* Output formatters
*/

void ConnectionPutLineEnd(PCONNECTION cn) {
	CPutChar(cn, '\r'); // CR
	CPutChar(cn, '\n'); // LF
}

VOID ConnectionPutString(PCONNECTION cn, const char *str) {
	int c;

	while ((c = *str++) != 0) CPutChar(cn, c);
}

VOID ConnectionPutStringFromWString(PCONNECTION cn, wchar_t *str) {
	int c;

	while ((c = *str++) != 0) CPutChar(cn, c);
}

void ConnectionPutInt(PCONNECTION cn, int num) {
	char ascii[32];

	ConnectionPutString(cn, _itoa(num, ascii, 10));
}

void ConnectionPutBytes(PCONNECTION cn, PCHAR buf, DWORD size) {
	int err = 0;
	WSABUF DataBuf;
	DataBuf.len = size;
	DataBuf.buf = cn->bufferOut;
	DWORD flags = 0;
	/*if ((WSASend(cn->socket, &DataBuf, 1, NULL, flags, &cn->ol, NULL) == SOCKET_ERROR) &&
		(WSA_IO_PENDING != (err = WSAGetLastError()))) {
		printf("WSASend failed with error: %d\n", err);
	}*/
	send(cn->socket, buf, size, 0);
}

BOOL ConnectionCopyFile(PCONNECTION cn, HANDLE hFile, int fSize) {
	CHAR data[BUFFERSIZE];
	// first send the file size in plain text
	ConnectionPutInt(cn, fSize);
	ConnectionPutLineEnd(cn);

	// flush buffer from previous writes
	ConnectionFlushBufferToSocket(cn);

	// Now copy the file in non-buffered mode
	while (fSize > 0) {
		DWORD toRead = min(BUFFERSIZE, fSize), read;
		if (!ReadFile(hFile, data, toRead, &read, NULL)) // synchronous read
			return FALSE; 
		ConnectionPutBytes(cn, data, read);
		fSize -= read;
	}

	return TRUE;

}

/*
 * A generic ( a la printf) generic output formatter
 */
void ConnectionPut(PCONNECTION cn, char* format, ...) {
	va_list ap;
	char *pcurr = format, c;

	va_start(ap, format);

	while ((c = *pcurr) != 0) {
		if (c != '%') {
			if (c != '\\')
				CPutChar(cn, c);
			else {
				c = *++pcurr;
				if (c == 0) break;
				if (c == 't')
					CPutChar(cn, '\t');
				else if (c == 'n')
					CPutChar(cn, '\n');
				else if (c == 'r')
					CPutChar(cn, '\r');
				else if (c == '0')
					CPutChar(cn, '\0');
			}
		}
		else {
			c = *++pcurr;
			if (c == 0) break;
			switch (c) {
			case 'd':
				ConnectionPutInt(cn, va_arg(ap, int));
				break;
			case 's':
				ConnectionPutString(cn, va_arg(ap, char *));
				break;
			case 'S':
				ConnectionPutStringFromWString(cn, va_arg(ap, wchar_t *));
				break;
			default:
				break;
			}

		}
		pcurr++;
	}
}


VOID ProcessCommand(PCONNECTION c) {
	c->activityState = SEND_RESPONSE;
	

	ProcessRequest(c);
	
}
VOID ProcessAnswerCompletion(PCONNECTION c)
{
	c->activityState = RECV_REQUEST;
	AsyncRead(c);
}

/*
* Connection initialization
*/

// Initialize a connection
VOID  ConnectionInit(PCONNECTION c, SOCKET s, sockaddr_in addr) {
	ZeroMemory(c, sizeof(Connection));
	c->socket = s;
	c->activityState = RECV_REQUEST;
	c->socketAddr = addr;
}

// Create (and initialize) new connection
PCONNECTION ConnectionCreate(SOCKET s, sockaddr_in addr) {
	PCONNECTION cn = (PCONNECTION)malloc(sizeof(CONNECTION));
	ConnectionInit(cn, s, addr);
	return cn;
}

// destroy a connection
VOID ConnectionDestroy(PCONNECTION cn) {

	printf("Destroying Connection to Client %X:%d\n", cn->socketAddr.sin_addr, cn->socketAddr.sin_port);
	shutdown(cn->socket, SD_BOTH);
	closesocket(cn->socket);
	free(cn);
}

 