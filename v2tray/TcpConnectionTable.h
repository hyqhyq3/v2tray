#pragma once
#include <map>

class TcpConnectionTable
{
public:
	TcpConnectionTable();

	std::string getAppName(uint32_t addr, uint32_t port);

	void update();

private:
	std::map<std::pair<uint32_t, uint32_t>, std::string> addrAppMap_;
	std::string getProcessName(uint32_t pid);
};

