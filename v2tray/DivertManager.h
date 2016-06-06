#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include <map>
#include "TcpConnectionTable.h"

struct Connection
{
	enum State {
		STATE_NOT_CONNECTED        = 0,
		STATE_SYN_SEEN             = 1,
		STATE_SYNACK_SEEN          = 2,
		STATE_ESTABLISHED          = 3,
		STATE_FIN_SEEN             = 4,
	};

	State state = STATE_NOT_CONNECTED;
	uint32_t srcPort = 0;
	uint32_t dstPort = 0;
	uint32_t srcAddr = 0;
	uint32_t dstAddr = 0;
};

class DivertManager
{
public:
	DivertManager();
	~DivertManager();
	void start();
	void stop();

	void worker();
	bool isEnabled();

private:
	std::atomic<bool> enabled_;
	std::thread thread_;
	HANDLE handle_;
	std::map<uint32_t, Connection> conns_;
	uint32_t proxyPort_;
	uint32_t proxyAddr_;
	TcpConnectionTable connTable_;
	typedef std::pair<uint32_t, uint32_t> Addr;
	struct Conn {
		Addr local;
		Addr remote;
		enum {
			NOT_CONNECTED,
			ESTABLISHED,
			TIME_WAIT,
		} state = NOT_CONNECTED;
	};
	std::map<Addr, Conn> srcDstMap_;
};

