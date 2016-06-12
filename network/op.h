#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <functional>

struct Op : OVERLAPPED
{
	Op() : OVERLAPPED{} {}
	std::function<void()> func_ptr;
};

