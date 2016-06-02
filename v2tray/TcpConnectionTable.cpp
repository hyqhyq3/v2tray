#include "stdafx.h"
#include "TcpConnectionTable.h"
#include <iphlpapi.h>
#include <iostream>
#include <winsock.h>
#include <Psapi.h>
#include <string>

TcpConnectionTable::TcpConnectionTable()
{

}

std::string TcpConnectionTable::getAppName(uint32_t addr, uint32_t port)
{
	auto key = std::make_pair(addr, port);
	if (addrAppMap_.count(key))
	{
		return addrAppMap_.at(key);
	}
	update();
	if (addrAppMap_.count(key))
	{
		return addrAppMap_.at(key);
	}
	return "";
}

void TcpConnectionTable::update()
{
	PMIB_TCPTABLE2 pTcpTable{ 0 };
	ULONG ulSize = sizeof(MIB_TCPTABLE2);
	DWORD dwRetVal;

	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) ==
		ERROR_INSUFFICIENT_BUFFER) {
		free(pTcpTable);
		pTcpTable = (MIB_TCPTABLE2 *)malloc(ulSize);
		if (pTcpTable == NULL) {
			printf("Error allocating memory\n");
			return ;
		}
	}

	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == NO_ERROR)
	{
		addrAppMap_.clear();
		for (int i = 0; i < pTcpTable->dwNumEntries; ++i)
		{
			std::string app = getProcessName(pTcpTable->table[i].dwOwningPid);
			addrAppMap_.emplace(std::make_pair(pTcpTable->table[i].dwLocalAddr, pTcpTable->table[i].dwLocalPort), app);
		}
	}
	else
	{
		std::cout << GetLastError() << std::endl;
	}

	free(pTcpTable);

}

std::string TcpConnectionTable::getProcessName(uint32_t pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, TRUE, pid);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		return "<unknown>";
	}
	char name[255]{};
	GetModuleBaseNameA(hProcess, NULL, name, sizeof(name));
	return name;
}
