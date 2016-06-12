#include "IOCP.h"
#include <iostream>
#include "util.h"

IOCP::IOCP()
{
	port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 10);
}

void IOCP::add(HANDLE handle)
{
	if (CreateIoCompletionPort(handle, port_, 0, 10) == NULL)
	{
		std::cout << getErrorString() << std::endl;
	}
}

OVERLAPPED* IOCP::getUpdates()
{ 
	DWORD n;
	ULONG_PTR key;
	OVERLAPPED* ol = NULL;
	GetQueuedCompletionStatus(port_, &n, &key, &ol, 100);
	return ol;
}
