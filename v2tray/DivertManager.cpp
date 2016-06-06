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
	proxyPort_ = htons(3000);
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
	handle_ = WinDivertOpen("tcp or udp.DstPort == 53", WINDIVERT_LAYER_NETWORK_FORWARD, 0, 0);
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
		WINDIVERT_UDPHDR* udpHdr = NULL;
		
		DivertHelperParsePacket(packet, len, &ipHdr, NULL, NULL, NULL, &tcpHdr, &udpHdr, NULL, NULL);
		if (ipHdr && udpHdr)
		{
			do {
				if (addr.Direction == WINDIVERT_DIRECTION_INBOUND)
				{
					break;
				}
				if (udpHdr->DstPort == htons(53))
				{
					std::cout << "dns query" << std::endl;
				}
			} while (0);
		}
		else if (ipHdr && tcpHdr)
		{
			do {

				if (addr.Direction == WINDIVERT_DIRECTION_INBOUND)
				{
					break;
				}
				if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && tcpHdr->SrcPort == proxyPort_)
				{
					Addr local = Addr(ipHdr->SrcAddr, tcpHdr->SrcPort); // trelay listen
					Addr remote = Addr(ipHdr->DstAddr, tcpHdr->DstPort); // target addr
					// 从代理返回的包
					if (srcDstMap_.count(remote))
					{
						const Conn& conn = srcDstMap_.at(remote);
						ipHdr->SrcAddr = remote.first;
						tcpHdr->SrcPort = remote.second;
						ipHdr->DstAddr = conn.local.first;
						tcpHdr->DstPort = conn.local.second;
					}

					std::cout << "forward packet for proxy " << addr2string(ipHdr->SrcAddr, tcpHdr->SrcPort) << " to " << addr2string(ipHdr->DstAddr, tcpHdr->DstPort) << "\n";
					addr.Direction = WINDIVERT_DIRECTION_INBOUND;
					break;
				}

				if (connTable_.getAppName(ipHdr->SrcAddr, tcpHdr->SrcPort) == "trelay.exe")
				{
					std::cout << "放行trelay.exe\n";
					break;
				}

				if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && tcpHdr->DstPort == htons(443))
				{
					// 直接转发到代理
					Addr local = Addr(ipHdr->SrcAddr, tcpHdr->SrcPort);
					Addr remote = Addr(ipHdr->DstAddr, tcpHdr->DstPort);
					if (tcpHdr->Fin || tcpHdr->Rst)
					{
						srcDstMap_.erase(remote);
					}
					else
					{
						Conn conn;
						conn.local = local;
						conn.remote = remote;
						srcDstMap_[conn.remote] = conn;
					}
					std::swap(ipHdr->SrcAddr, ipHdr->DstAddr);
					tcpHdr->SrcPort = tcpHdr->DstPort;
					tcpHdr->DstPort = proxyPort_;

					std::cout << "map size : " << srcDstMap_.size() << " \n";
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
