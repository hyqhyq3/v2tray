#pragma once
#include "Address.h"
#include "winsock2.h"
#include <iosfwd>

class Endpoint
{
public:
	Endpoint();
	explicit Endpoint(const sockaddr_in* in);
	Endpoint(const Address& a, int p);
	Address getAddress() const;
	int getPort() const;

private:
	Address address_;
	int port_;
};

std::ostream& operator<<(std::ostream& os, const Endpoint& ep);
