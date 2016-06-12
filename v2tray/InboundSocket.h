#pragma once
#include "network/Socket.h"

class InboundSocket
	: public Socket
{
public:
	using Socket::Socket;
	virtual void handleRead(const char* data, size_t size) override;

};

