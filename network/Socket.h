#pragma once
#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#include "op.h"
#include "Endpoint.h"
#include "Noncopyable.h"

class Socket
	: Noncopyable
{
public:
	Socket();
	explicit Socket(int sockfd);
	Socket(int sockfd, const Endpoint& peerEndpoint);
	virtual ~Socket();
	void start();
	int getSockFd();

	Endpoint getLocalEndpoint() const;
	Endpoint getRemoteEndpoint() const;

protected:
	virtual void handleRead(const char* data, size_t size) = 0;

private:
	void init();
	void onRead(Op* op);
private:
	int sockfd_;
	char buffer_[512];
	WSABUF wsabuf_;
	DWORD flags_;
	DWORD received_;
	bool hasPeerEndpoint_;
	Endpoint peerEndpoint_;
};
