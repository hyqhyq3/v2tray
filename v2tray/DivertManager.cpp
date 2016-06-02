#include "stdafx.h"
#include "DivertManager.h"
#include "windivert.h"
#include <iostream>
#include <winsock.h>
#include "v2tray.h"
#include <vector>
#include <sstream>
#include <iterator>

DivertManager::DivertManager()
{
	proxyPort_ = htons(4411);
}

DivertManager::~DivertManager()
{
	stop();
}

void DivertManager::start()
{
	showConsole();
	thread_ = std::thread(std::bind(&DivertManager::worker, this));
}

void DivertManager::stop()
{
	if (enabled_)
	{
		enabled_ = false;
		thread_.join();
		handle_ = NULL;
	}
}

static std::string addr2string(uint32_t addr, uint32_t port)
{
	std::stringstream ss;
	in_addr a;
	a.S_un.S_addr = addr;
	ss << inet_ntoa(a);
	ss << ":" << ntohs(port);
	return ss.str();
}

void DivertManager::worker()
{
	handle_ = WinDivertOpen("tcp", WINDIVERT_LAYER_NETWORK, 0, 0);
	if (handle_ == INVALID_HANDLE_VALUE)
	{
		std::cout << "WinDivertOpen :" << GetLastError() <<std::endl;
		return;
	}
	enabled_ = true;
	char packet[10000];
	WINDIVERT_ADDRESS addr;
	uint32_t len;
	while (enabled_)
	{
		if (!WinDivertRecv(handle_, packet, sizeof(packet), &addr, &len))
		{
			continue;
		}
		WINDIVERT_IPHDR* ipHdr = NULL;
		WINDIVERT_TCPHDR* tcpHdr = NULL;
		
		DivertHelperParsePacket(packet, len, &ipHdr, NULL, NULL, NULL, &tcpHdr, NULL, NULL, NULL);

		if (ipHdr && tcpHdr)
		{
			do {
				if (addr.Direction == WINDIVERT_DIRECTION_INBOUND)
				{
					break;
				}
				if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && tcpHdr->SrcPort == proxyPort_)
				{
					// 从代理返回的包
					std::swap(ipHdr->DstAddr, ipHdr->SrcAddr);
					tcpHdr->SrcPort = htons(443);

					std::cout << "forward packet for proxy " << addr2string(ipHdr->SrcAddr, tcpHdr->SrcPort) << " to " << addr2string(ipHdr->DstAddr, tcpHdr->DstPort) << "\n";
					addr.Direction = WINDIVERT_DIRECTION_INBOUND;
					break;
				}

				if (connTable_.getAppName(ipHdr->SrcAddr, tcpHdr->SrcPort) == "proxy.exe")
				{
					std::cout << "放行proxy.exe\n";
					break;
				}

				if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && tcpHdr->DstPort == htons(443))
				{
					// 直接转发到代理
					tcpHdr->DstPort = proxyPort_;
					std::swap(ipHdr->SrcAddr, ipHdr->DstAddr);

					std::cout << "forward packet for 443 " << addr2string(ipHdr->SrcAddr, tcpHdr->SrcPort) << " to " << addr2string(ipHdr->DstAddr, tcpHdr->DstPort) << "\n";
					addr.Direction = WINDIVERT_DIRECTION_INBOUND;
					break;
				}
			} while (0);

		}

		DivertHelperCalcChecksums(packet, len, 0);

		if (!WinDivertSend(handle_, packet, len, &addr, NULL))
		{
			continue;
		}
	}
	WinDivertClose(handle_);
}

bool DivertManager::isEnabled()
{
	return enabled_;
}
