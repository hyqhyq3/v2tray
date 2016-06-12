#pragma once
#include <stdint.h>
#include "Noncopyable.h"

class IOCP;
class Endpoint;

class Server
	: Noncopyable
{
public:
	Server(int port);
	virtual ~Server();
	virtual void init() = 0;
	virtual void onDestroy() = 0;
	virtual void handleConnection(int sockfd, const Endpoint& remoteEndpoint) = 0;

	void start();
	void stop();
	void startAccept();
	void onAccept();
private:
	int port_;
	IOCP* iocp_;
	bool running_;
	int listenSocket_;
	int acceptSocket_;
	char buffer_[512];
	unsigned long received_;
};

