#include "Server.h"
#include "IOCP.h"
#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#include <ws2tcpip.h>
#include "mswsock.h"
#include <functional>
#include <iostream>
#include "util.h"
#include "op.h"
#include "Endpoint.h"

Server::Server(int port)
	: port_(port)
{
	iocp_ = new IOCP();
	listenSocket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	sockaddr_in in{};
	in.sin_family = AF_INET;
	in.sin_port = htons(port);
	in.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listenSocket_, (sockaddr*)&in, sizeof(in)) == -1) {
		std::cout << "bind error " << getErrorString() << std::endl;
	}
	listen(listenSocket_, 10);
	iocp_->add((HANDLE)listenSocket_);
}

Server::~Server()
{
	delete iocp_;
}

void Server::start()
{
	init();
	startAccept();
	running_ = true;
	while (running_) {
		OVERLAPPED* ol = iocp_->getUpdates();
		if (ol != NULL)
		{
			Op* op = static_cast<Op*>(ol);
			op->func_ptr();
			delete op;
		}
	}
	onDestroy();
	delete this;
}

void Server::stop()
{
	running_ = false;
}

void Server::startAccept()
{
	const int sockaddr_size = sizeof(sockaddr_in);
	acceptSocket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	iocp_->add((HANDLE)acceptSocket_);
	Op* op = new Op();
	op->func_ptr = std::bind(&Server::onAccept, this);
	AcceptEx(listenSocket_, acceptSocket_, buffer_, 0, sockaddr_size + 16, sockaddr_size + 16, &received_, op);
	std::cout << getErrorString() << std::endl;
}

void Server::onAccept()
{
	const int sockaddr_size = sizeof(sockaddr_in) + 16;
	sockaddr_in *local,*remote;
	int locallen, peerlen;
	GetAcceptExSockaddrs(buffer_, 0, sockaddr_size, sockaddr_size, (sockaddr**)&local, &locallen, (sockaddr**)&remote, &peerlen);
	setsockopt(acceptSocket_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket_, sizeof(SOCKET));
	handleConnection(acceptSocket_, Endpoint(remote));
	startAccept();
}