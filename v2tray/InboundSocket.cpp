#include "stdafx.h"
#include "InboundSocket.h"
#include <iostream>
#include <string>

void InboundSocket::handleRead(const char* data, size_t size)
{
	std::cout << "read [" << std::string(data, size) << "]" << std::endl;
}
