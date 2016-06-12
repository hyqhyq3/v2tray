#include "Address.h"
#include <ws2tcpip.h>



Address::Address()
	: addr_(0)
{

}

Address Address::v4(int a, int b, int c, int d)
{
	Address addr;
	addr.addr_ = ((uint8_t)a << 24) | ((uint8_t)b << 16) | ((uint8_t)c << 8) | d;
	return addr;
}

Address Address::v4(unsigned long hl)
{
	Address addr;
	addr.addr_ = hl;
	return addr;
}

unsigned long Address::toUlong()
{
	return addr_;
}

std::string Address::toString() const
{
	sockaddr_in in{};
	in.sin_family = AF_INET;
	in.sin_addr.S_un.S_addr = addr_;
	char buf[1024]{};
	size_t size = 1024;
	::inet_ntop(AF_INET, (void*)&addr_, buf, size);
	return buf;
}

std::ostream& operator<<(std::ostream& os, const Address& a)
{
	return os << a.toString();
}
