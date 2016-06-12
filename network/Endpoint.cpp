#include "Endpoint.h"
#include <winsock.h>

Endpoint::Endpoint()
	: port_(0)
{

}


Endpoint::Endpoint(const Address& a, int p)
{
	address_ = a;
	port_ = htons(p);
}

Endpoint::Endpoint(const sockaddr_in* in)
{
	address_ = Address::v4(in->sin_addr.S_un.S_addr);
	port_ = in->sin_port;
}

Address Endpoint::getAddress() const
{
	return address_;
}

int Endpoint::getPort() const
{
	return ntohs(port_);
}

std::ostream& operator<<(std::ostream& os, const Endpoint& ep)
{
	return os << ep.getAddress() << ":" << ep.getPort();
}