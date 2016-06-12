#include "stdafx.h"
#include "RelayServer.h"
#include "InboundSocket.h"
#include <iostream>


RelayServer::RelayServer()
	: Server(3123)
{

}

void RelayServer::init()
{
}

void RelayServer::onDestroy()
{
}

void RelayServer::handleConnection(int sockfd, const Endpoint& remoteEndpoint)
{
	InboundSocket* s = new InboundSocket(sockfd, remoteEndpoint);
	s->start();
	std::cout << remoteEndpoint << std::endl;
	std::cout << s->getLocalEndpoint() << std::endl;
	std::cout << s->getRemoteEndpoint() << std::endl;
}
