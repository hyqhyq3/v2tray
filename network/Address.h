#pragma once
#include <stdint.h>
#include <string>

class Address
{
public:
	Address();
	static Address v4(int a, int b, int c, int d);
	static Address v4(unsigned long hl);

	unsigned long toUlong();

	std::string toString() const;

private:
	uint32_t addr_;
};

std::ostream& operator<<(std::ostream& os, const Address& a);

