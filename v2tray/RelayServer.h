#pragma once
#include "network/Server.h"

class RelayServer
	: public Server
{
public:
	RelayServer();

	virtual void init() override;

	virtual void onDestroy() override;

	virtual void handleConnection(int sockfd, const Endpoint& remoteEndpoint) override;

};

