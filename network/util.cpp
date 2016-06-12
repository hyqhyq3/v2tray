#include "util.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"



std::string getErrorString()
{
	DWORD code = GetLastError();
	char msg[256]{};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 256, NULL);
	return msg;
}
