// trelay.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

enum {
	COMPLETION_KEY_NONE,
	COMPLETION_KEY_IO
};

std::string getError()
{
	wchar_t buf[256];
	DWORD dwSize;
	DWORD dwError = GetLastError();
	dwSize = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	char str[256];
	dwSize = WideCharToMultiByte(CP_OEMCP, 0, buf, dwSize, str, 256, "?", NULL);
	str[dwSize] = 0;
	return str;
}

DWORD WINAPI IocpThread(LPVOID lParam)
{
	HANDLE hIocp = (HANDLE)lParam;
	DWORD cbBytes{};
	ULONG_PTR pulKey{};
	OVERLAPPED* pOverlapped{};
	for (;;) {
		GetQueuedCompletionStatus(hIocp, &cbBytes, &pulKey, &pOverlapped, INFINITE);
		std::cout << "get event" << std::endl;
	}
	return 0;
}

int main()
{
	WSAData wsaData{};
	WSAStartup(0x0202, &wsaData);

	HANDLE hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 10);

	HANDLE hThread = CreateThread(NULL, 0, &IocpThread, hIocp, 0, NULL);

	SOCKET sListen = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	sockaddr_in addr{};
	addr.sin_family = PF_INET;
	addr.sin_port = htons(4000);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	size_t len = sizeof(addr);
	int ret = bind(sListen, (sockaddr*)&addr, len);
	if (ret == -1) {
		std::cerr << getError() << std::endl;
		return 0;
	}

	if ((ret = listen(sListen, 10)) == -1)
	{
		std::cerr << getError() << std::endl;
		return 0;
	}

	hIocp = CreateIoCompletionPort((HANDLE)sListen, hIocp, 0, 0);

	SOCKET sAccept = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	
	hIocp = CreateIoCompletionPort((HANDLE)sAccept, hIocp, COMPLETION_KEY_IO, 0);
	BYTE addrBlock[1024];
	DWORD received{};
	OVERLAPPED ol{};
	ret = AcceptEx(sListen, sAccept, addrBlock, 1024 - (sizeof(sockaddr_in) + 16)*2, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &received, &ol);
	std::cerr << getError() << std::endl;
	WaitForSingleObject(hThread, INFINITE);
	
    return 0;
}

