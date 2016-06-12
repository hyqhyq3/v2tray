#include "Socket.h"
#include "winsock2.h"
#include <functional>
#include "op.h"
#include "util.h"
#include <iostream>

Socket::Socket()
{
	sockfd_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	init();
}

Socket::Socket(int sockfd)
{
	sockfd_ = sockfd;
	init();
}

Socket::Socket(int sockfd, const Endpoint& peerEndpoint)
	: sockfd_(sockfd)
	, hasPeerEndpoint_(true)
	, peerEndpoint_(peerEndpoint)
{
	
}

Socket::~Socket()
{
	closesocket(sockfd_);
}

void Socket::start()
{
	Op* op = new Op();
	op->func_ptr = std::bind(&Socket::onRead, this, op);
	DWORD received = 0;
	DWORD flags = 0 ;
	if (WSARecv(sockfd_, &wsabuf_, 1, &received, &flags, op, NULL))
	{
		std::cout << "recv " << getErrorString() << std::endl;
	}
}

int Socket::getSockFd()
{
	return sockfd_;
}

Endpoint Socket::getLocalEndpoint() const
{
	sockaddr_in in{};
	int len = sizeof(in);
	if (getsockname(sockfd_, (sockaddr*)&in, &len))
	{
		std::cout << getErrorString() << std::endl;
	}
	return Endpoint(&in);
}

Endpoint Socket::getRemoteEndpoint() const
{
	if (hasPeerEndpoint_)
	{
		return peerEndpoint_;
	}
	else
	{
		sockaddr_in in{};
		int len = sizeof(in);
		if (getpeername(sockfd_, (sockaddr*)&in, &len))
		{
			std::cout << getErrorString() << std::endl;
		}
		return Endpoint(Address::v4(ntohl(in.sin_addr.S_un.S_addr)), htons(in.sin_port));
	}

}

void Socket::init()
{
	wsabuf_.len = 10;
	wsabuf_.buf = buffer_;
}

void Socket::onRead(Op* op)
{
	handleRead(buffer_, op->InternalHigh);
	start();
}
