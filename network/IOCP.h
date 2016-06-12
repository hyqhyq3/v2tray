#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

class IOCP
{
public:
	IOCP();
	void add(HANDLE handle);
	OVERLAPPED* getUpdates();
private:
	HANDLE port_;
};

