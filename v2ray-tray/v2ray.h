#pragma once
#include <windef.h>
#include <string>

class V2Ray
{
public:
	~V2Ray();
	void init(HWND hWnd, std::string configFile);
	void restart();
	void start();
	void kill();
	void editConfig();
	void setProxy();
	void setNoProxy();
private:
	HWND m_hWnd;
	HANDLE m_hProcess;
};

